#include "render/linux/vulkanrenderer.h"
#include "ecs/component/meshcomponent.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "platform/linux/platformlinux.h"
#include "render/linux/vulkanhelpers.h"
#include "buffer/ubuffer.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <spdlog/spdlog.h>
#include <cstring>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static bool enableValidationLayers()
{
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}

bool VulkanRenderer::init(IPlatform *platform)
{
    spdlog::debug("Initialising Vulkan renderer");
    platform_ = platform;
    width_ = platform->getConfig().window_width;
    height_ = platform->getConfig().window_height;

    try
    {
        createInstance();
        setupDebugMessenger();
        createSurface();
        selectGPU();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();

        if (!initImGui())
            throw std::runtime_error("ImGui failed to initialise");

        createSynchronizationPrimitives();

        spdlog::info("VulkanRenderer initialised successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("VulkanRenderer init failed: {}", e.what());
        cleanup();
        return false;
    }
}

void VulkanRenderer::createInstance()
{
    spdlog::debug("Creating Vulkan instance");
    if (enableValidationLayers() && !checkValidationLayerSupport())
    {
        throw std::runtime_error(
            "Validation layers requested but not available"
        );
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Iron Cube";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Iron Cube Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char *> extensions = getRequiredExtensions();
    spdlog::info("Vulkan instance extensions ({}):", extensions.size());
    for (const char *&ext : extensions)
    {
        spdlog::info("  - {}", ext);
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    std::vector<const char *> layers;
    if (enableValidationLayers())
    {
        layers = {"VK_LAYER_KHRONOS_validation"};
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    createInfo.pNext = nullptr;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    vkCheck(result, "Failed to create Vulkan instance");
}

void VulkanRenderer::setupDebugMessenger()
{
    spdlog::debug("Setting up debug messenger");
    if (!enableValidationLayers())
        return;
    debugMessenger_ = createDebugMessenger(instance_);
}

void VulkanRenderer::createSurface()
{
    spdlog::debug("Creating surface");
    auto *glfwWindow = static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    VkResult result =
        glfwCreateWindowSurface(instance_, glfwWindow, nullptr, &surface_);
    vkCheck(result, "Failed to create window surface");
}

void VulkanRenderer::selectGPU()
{
    spdlog::debug("Selecting GPU");
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance_, &gpuCount, nullptr);

    if (gpuCount == 0)
    {
        throw std::runtime_error("No GPUs with Vulkan support found");
    }

    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance_, &gpuCount, gpus.data());

    bool found = false;
    for (VkPhysicalDevice_T *&gpu : gpus)
    {
        if (isDeviceSuitable(gpu))
        {
            physicalDevice_ = gpu;
            found = true;
            break;
        }
    }

    if (!found)
    {
        throw std::runtime_error("No suitable GPU found");
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice_, &props);
    spdlog::info("Selected GPU: {}", props.deviceName);
}

void VulkanRenderer::createLogicalDevice()
{
    spdlog::debug("Creating logical device");
    uint32_t queueIndex = findQueueFamilyIndex();

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 1;
    const char *ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &ext;

    VkResult result =
        vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_);
    vkCheck(result, "Failed to create logical device");

    vkGetDeviceQueue(device_, queueIndex, 0, &graphicsQueue_);
}

void VulkanRenderer::createSwapchain()
{
    spdlog::debug("Creating swap chain");
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice_, surface_, &capabilities
    );

    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice_, surface_, &formatCount, nullptr
    );
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice_, surface_, &formatCount, formats.data()
    );

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice_, surface_, &presentModeCount, nullptr
    );
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice_, surface_, &presentModeCount, presentModes.data()
    );

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    VkExtent2D extent = chooseSwapExtent(capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface_;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(
        device_, &swapchainCreateInfo, nullptr, &swapchain_
    );
    vkCheck(result, "Failed to create swap chain");

    swapchainImageFormat_ = surfaceFormat.format;
    swapchainExtent_ = extent;

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(
        device_, swapchain_, &imageCount, swapchainImages_.data()
    );
}

void VulkanRenderer::createImageViews()
{
    spdlog::debug("Creating image views");
    swapchainImageViews_.resize(swapchainImages_.size());

    for (size_t i = 0; i < swapchainImages_.size(); i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages_[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchainImageFormat_;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(
            device_, &viewInfo, nullptr, &swapchainImageViews_[i]
        );
        vkCheck(result, "Failed to create image view");
    }
}

void VulkanRenderer::createRenderPass()
{
    spdlog::debug("Creating render pass");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result =
        vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_);
    vkCheck(result, "Failed to create render pass");
}

void VulkanRenderer::createGraphicsPipeline(const ShaderPipelineConfig &config)
{
    spdlog::debug("Creating graphics pipeline");
    currentPipelineConfig_ = config;

    // Create shader instances from definitions
    shaders_.clear();
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (const auto &shaderDef : config.shaders)
    {
        shaders_.push_back(
            std::make_unique<Shader>(
                device_, shaderDef.path, shaderDef.stage, shaderDef.entryPoint
            )
        );
        shaderStages.push_back(shaders_.back()->getShaderStageInfo());
    }

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    for (const auto &vb : config.vertexBindings)
    {
        VkVertexInputBindingDescription desc{};
        desc.binding = vb.binding;
        desc.stride = vb.stride;
        desc.inputRate = vb.inputRate;
        bindingDescriptions.push_back(desc);
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    for (const auto &va : config.vertexAttributes)
    {
        VkVertexInputAttributeDescription desc{};
        desc.binding = 0;
        desc.location = va.location;
        desc.format = va.format;
        desc.offset = va.offset;
        attributeDescriptions.push_back(desc);
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInput.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInput.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = config.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = config.polygonMode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = config.cullMode;
    rasterizer.frontFace = config.frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = config.samples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(config.dynamicStates.size());
    dynamicState.pDynamicStates = config.dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (const auto &db : config.descriptorBindings)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = db.binding;
        binding.descriptorType = db.descriptorType;
        binding.descriptorCount = db.descriptorCount;
        binding.stageFlags = db.stageFlags;
        binding.pImmutableSamplers = nullptr;
        layoutBindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    vkCheck(
        vkCreateDescriptorSetLayout(
            device_, &layoutInfo, nullptr, &descriptorSetLayout_
        ),
        "Failed to create descriptor set layout"
    );

    VkResult result = vkCreatePipelineLayout(
        device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_
    );
    vkCheck(result, "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.layout = pipelineLayout_;

    result = vkCreateGraphicsPipelines(
        device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_
    );
    vkCheck(result, "Failed to create graphics pipeline");
}

void VulkanRenderer::createFramebuffers()
{
    spdlog::debug("Creating framebuffers");
    framebuffers_.resize(swapchainImageViews_.size());

    for (size_t i = 0; i < swapchainImageViews_.size(); i++)
    {
        VkImageView attachments[] = {swapchainImageViews_[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent_.width;
        framebufferInfo.height = swapchainExtent_.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(
            device_, &framebufferInfo, nullptr, &framebuffers_[i]
        );
        vkCheck(result, "Failed to create framebuffer");
    }
}

void VulkanRenderer::createCommandPool()
{
    spdlog::debug("Creating command pool");
    uint32_t queueIndex = findQueueFamilyIndex();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult result =
        vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_);
    vkCheck(result, "Failed to create command pool");
}

void VulkanRenderer::createCommandBuffers()
{
    spdlog::debug("Creating command buffers");
    commandBuffers_.resize(framebuffers_.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        static_cast<uint32_t>(commandBuffers_.size());

    VkResult result =
        vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data());
    vkCheck(result, "Failed to allocate command buffers");
}

void VulkanRenderer::createUniformBuffer()
{
    spdlog::debug("Creating uniform buffer");
    VkDeviceSize bufferSize = sizeof(UBO);

    uniformAlloc_ = allocateBuffer(
        bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    if (uniformAlloc_.buffer == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to allocate uniform buffer");
}

BufferAllocation VulkanRenderer::allocateBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
)
{
    BufferAllocation alloc{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result =
        vkCreateBuffer(device_, &bufferInfo, nullptr, &alloc.buffer);
    vkCheck(result, "Failed to create buffer");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, alloc.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device_, &allocInfo, nullptr, &alloc.memory);
    vkCheck(result, "Failed to allocate buffer memory");

    vkBindBufferMemory(device_, alloc.buffer, alloc.memory, 0);
    alloc.size = size;
    return alloc;
}

void *VulkanRenderer::mapMemory(BufferAllocation &alloc, VkDeviceSize size)
{
    void *data;
    VkResult result = vkMapMemory(device_, alloc.memory, 0, size, 0, &data);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to map memory");
        return nullptr;
    }
    return data;
}

void VulkanRenderer::unmapMemory(BufferAllocation &alloc)
{
    vkUnmapMemory(device_, alloc.memory);
}

void VulkanRenderer::freeBuffer(BufferAllocation &alloc)
{
    if (alloc.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device_, alloc.buffer, nullptr);
        alloc.buffer = VK_NULL_HANDLE;
    }
    if (alloc.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device_, alloc.memory, nullptr);
        alloc.memory = VK_NULL_HANDLE;
    }
}

BufferAllocation *VulkanRenderer::allocateVertexBufferForGeometry(
    Geometry *geometry, uint32_t vertexCount
)
{
    // Check if we already have a buffer for this geometry
    auto it = vertexAllocs_.find(geometry);
    if (it != vertexAllocs_.end())
    {
        // Reuse existing buffer if size matches
        VkDeviceSize currentSize = it->second.size;
        VkDeviceSize neededSize = sizeof(Vertex) * vertexCount;
        if (currentSize >= neededSize)
        {
            return &it->second;
        }

        // Need to resize - wait for GPU, free old, allocate new
        VkResult waitResult = vkDeviceWaitIdle(device_);
        if (waitResult != VK_SUCCESS && waitResult != VK_ERROR_DEVICE_LOST)
        {
            throw std::runtime_error(
                std::string("Device wait idle failed") +
                " (VkResult: " + std::to_string(waitResult) + ")"
            );
        }
        freeBuffer(it->second);
    }

    VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
    if (bufferSize > 10000)
        return nullptr;
    spdlog::info(
        "Allocating vertex buffer: {} vertices, {} bytes", vertexCount,
        bufferSize
    );

    BufferAllocation alloc = allocateBuffer(
        bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    if (alloc.buffer == VK_NULL_HANDLE)
        return nullptr;

    vertexAllocs_[geometry] = alloc;
    return &vertexAllocs_[geometry];
}

void VulkanRenderer::createDescriptorSet()
{
    spdlog::debug("Creating descriptor set");
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    poolInfo.flags = 0;

    vkCheck(
        vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_),
        "Failed to create descriptor pool"
    );

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;

    vkCheck(
        vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet_),
        "Failed to allocate descriptor sets"
    );

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformAlloc_.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UBO);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet_;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
}

void VulkanRenderer::createSynchronizationPrimitives()
{
    spdlog::debug("Creating synchronization primitives");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(
        device_, &semaphoreInfo, nullptr, &imageAvailableSemaphore_
    );
    vkCreateSemaphore(
        device_, &semaphoreInfo, nullptr, &renderFinishedSemaphore_
    );

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFence_);
}

bool VulkanRenderer::initialisePipeline(const ShaderPipelineConfig &config)
{
    spdlog::debug("Initialising graphics pipelines");
    try
    {
        createGraphicsPipeline(config);
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createUniformBuffer();
        createDescriptorSet();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Pipeline initialisation failed: {}", e.what());
        return false;
    }

    spdlog::info("Graphics pipelines initialised");
    return true;
}

void VulkanRenderer::setExitRequestedCallback(std::function<bool()> callback)
{
    exitRequested_ = callback;
}

void VulkanRenderer::setDeltaTime(float deltaSeconds)
{
    deltaTime_ = deltaSeconds;
}

bool VulkanRenderer::beginFrame()
{
    if (exitRequested_ && exitRequested_())
        return false;

    VkResult result = vkAcquireNextImageKHR(
        device_, swapchain_, kGpuOperationTimeoutNs, imageAvailableSemaphore_,
        VK_NULL_HANDLE, &currentImageIndex_
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vkDestroySemaphore(device_, imageAvailableSemaphore_, nullptr);
        vkDestroySemaphore(device_, renderFinishedSemaphore_, nullptr);
        vkDestroyFence(device_, inFlightFence_, nullptr);
        imageAvailableSemaphore_ = VK_NULL_HANDLE;
        renderFinishedSemaphore_ = VK_NULL_HANDLE;
        inFlightFence_ = VK_NULL_HANDLE;
        return true;
    }
    vkCheck(result, "Failed to acquire swap chain image");

    vkWaitForFences(
        device_, 1, &inFlightFence_, VK_TRUE, kGpuOperationTimeoutNs
    );
    vkResetFences(device_, 1, &inFlightFence_);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkCheck(
        vkBeginCommandBuffer(commandBuffers_[currentImageIndex_], &beginInfo),
        "Failed to begin command buffer"
    );

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = framebuffers_[currentImageIndex_];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent_;

    VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(
        commandBuffers_[currentImageIndex_], &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );
    vkCmdBindPipeline(
        commandBuffers_[currentImageIndex_], VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline_
    );

    vkCmdBindDescriptorSets(
        commandBuffers_[currentImageIndex_], VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout_, 0, 1, &descriptorSet_, 0, nullptr
    );

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainExtent_.width);
    viewport.height = static_cast<float>(swapchainExtent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers_[currentImageIndex_], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent_;
    vkCmdSetScissor(commandBuffers_[currentImageIndex_], 0, 1, &scissor);
    return true;
}

bool VulkanRenderer::renderScene(Scene *scene)
{
    if (!scene)
        return false;

    for (const std::unique_ptr<Entity> &entity : scene->getEntities())
    {
        MeshComponent *meshComponent = entity->getComponent<MeshComponent>();
        if (meshComponent)
        {
            meshComponent->render();
        }
    }

    return true;
}

bool VulkanRenderer::endFrame()
{
    if (exitRequested_ && exitRequested_())
        return false;

    if (imageAvailableSemaphore_ == VK_NULL_HANDLE)
        return false;

    vkCmdEndRenderPass(commandBuffers_[currentImageIndex_]);
    vkCheck(
        vkEndCommandBuffer(commandBuffers_[currentImageIndex_]),
        "Failed to end command buffer"
    );

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphore_;
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers_[currentImageIndex_];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphore_;

    VkResult result =
        vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFence_);
    vkCheck(result, "Failed to submit draw command buffer");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphore_;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &currentImageIndex_;

    vkCheck(
        vkQueuePresentKHR(graphicsQueue_, &presentInfo),
        "Failed to present frame"
    );

    return true;
}

void VulkanRenderer::setUniformModelMatrix(glm::mat4 model)
{
    currentModelMatrix_ = model;
    uniformDirty_ = true;
}

bool VulkanRenderer::drawGeometry(Geometry *geometry)
{
    if (!geometry || !geometry->vertices || geometry->vertexCount == 0)
        return false;

    // Allocate or reuse vertex buffer for this geometry
    BufferAllocation *vertAlloc =
        allocateVertexBufferForGeometry(geometry, geometry->vertexCount);
    if (!vertAlloc)
        return false;

    // Upload vertex data
    VkDeviceSize bufferSize = sizeof(Vertex) * geometry->vertexCount;
    void *mapped = mapMemory(*vertAlloc, bufferSize);
    if (mapped)
    {
        std::memcpy(
            mapped, geometry->vertices->data(), static_cast<size_t>(bufferSize)
        );
        unmapMemory(*vertAlloc);
    }

    // Update uniform buffer if dirty
    if (uniformDirty_)
    {
        UBO ubo{};
        std::memcpy(
            ubo.model, glm::value_ptr(currentModelMatrix_), 16 * sizeof(float)
        );

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        std::memcpy(ubo.view, glm::value_ptr(view), 16 * sizeof(float));

        float aspect = swapchainExtent_.width /
                       static_cast<float>(swapchainExtent_.height);
        glm::mat4 proj =
            glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
        proj[1][1] *= -1.0f;
        std::memcpy(ubo.proj, glm::value_ptr(proj), 16 * sizeof(float));

        VkDeviceSize uboSize = sizeof(UBO);
        void *uboMapped = mapMemory(uniformAlloc_, uboSize);
        if (uboMapped)
        {
            std::memcpy(uboMapped, &ubo, sizeof(UBO));
            unmapMemory(uniformAlloc_);
        }
        uniformDirty_ = false;
    }

    // Bind vertex buffer and draw
    VkBuffer vertexBuffers[] = {vertAlloc->buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(
        commandBuffers_[currentImageIndex_], 0, 1, vertexBuffers, offsets
    );
    vkCmdDraw(
        commandBuffers_[currentImageIndex_], geometry->vertexCount, 1, 0, 0
    );

    return true;
}

void VulkanRenderer::swapBuffers()
{
}

bool VulkanRenderer::initImGui()
{
    imguiContext_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext_);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    GLFWwindow *glfwWindow =
        static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.poolSizeCount =
        static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
    poolInfo.pPoolSizes = poolSizes;

    VkResult result = vkCreateDescriptorPool(
        device_, &poolInfo, nullptr, &imguiDescriptorPool_
    );
    if (result != VK_SUCCESS)
    {
        spdlog::error(
            "Failed to create ImGui descriptor pool: {}",
            static_cast<int>(result)
        );
        imguiDescriptorPool_ = VK_NULL_HANDLE;
        return false;
    }

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance_;
    initInfo.PhysicalDevice = physicalDevice_;
    initInfo.Device = device_;
    initInfo.QueueFamily = findQueueFamilyIndex();
    initInfo.Queue = graphicsQueue_;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = imguiDescriptorPool_;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(swapchainImages_.size());
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass = renderPass_;

    ImGui_ImplVulkan_Init(&initInfo);

    // Upload font texture
    ImGui_ImplVulkan_CreateFontsTexture();

    spdlog::info("ImGui initialised");
    return true;
}

bool VulkanRenderer::newFrame()
{
    if (exitRequested_ && exitRequested_())
        return false;

    ImGuiIO &io = ImGui::GetIO();

    // Setup display size and framebuffer scale
    io.DisplaySize = ImVec2(
        static_cast<float>(swapchainExtent_.width),
        static_cast<float>(swapchainExtent_.height)
    );
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DeltaTime = deltaTime_;

    // Platform/Renderer backend new frame
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    return true;
}

bool VulkanRenderer::renderImGui()
{
    if (exitRequested_ && exitRequested_())
        return false;

    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    if (drawData->TotalVtxCount > 0)
    {
        ImGui_ImplVulkan_RenderDrawData(
            drawData, commandBuffers_[currentImageIndex_]
        );
    }

    return true;
}

void VulkanRenderer::shutdownImGui()
{
    if (imguiContext_)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imguiContext_);
        imguiContext_ = nullptr;

        if (imguiDescriptorPool_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device_, imguiDescriptorPool_, nullptr);
            imguiDescriptorPool_ = VK_NULL_HANDLE;
        }

        spdlog::info("ImGui shut down");
    }
}

void VulkanRenderer::shutdown()
{
    spdlog::info("VulkanRenderer shutting down");

    // Wait for GPU to finish all work BEFORE destroying any resources
    if (device_ != VK_NULL_HANDLE)
    {
        VkResult waitResult = vkDeviceWaitIdle(device_);
        if (waitResult != VK_SUCCESS && waitResult != VK_ERROR_DEVICE_LOST)
        {
            spdlog::error(
                "Device wait idle failed (VkResult: {})",
                static_cast<int>(waitResult)
            );
        }
    }

    shutdownImGui();
    cleanup();
}

void VulkanRenderer::cleanup()
{
    // Destroy shader modules before the device is destroyed.
    // Shader::~Shader calls vkDestroyShaderModule(device_) so device_ must
    // still be valid.
    shaders_.clear();

    if (device_ != VK_NULL_HANDLE)
    {
        for (VkFramebuffer_T *&framebuffer : framebuffers_)
            vkDestroyFramebuffer(device_, framebuffer, nullptr);

        for (VkImageView_T *&imageView : swapchainImageViews_)
            vkDestroyImageView(device_, imageView, nullptr);

        // Cleanup vertex buffers
        for (auto &[geom, alloc] : vertexAllocs_)
        {
            if (alloc.buffer != VK_NULL_HANDLE)
            {
                freeBuffer(alloc);
            }
        }
        vertexAllocs_.clear();
        // Cleanup uniform buffer
        if (uniformAlloc_.buffer != VK_NULL_HANDLE)
        {
            freeBuffer(uniformAlloc_);
            uniformAlloc_ = BufferAllocation{};
        }
        if (descriptorPool_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
            descriptorPool_ = VK_NULL_HANDLE;
        }
        if (descriptorSetLayout_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(
                device_, descriptorSetLayout_, nullptr
            );
            descriptorSetLayout_ = VK_NULL_HANDLE;
        }
        if (graphicsPipeline_ != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
            graphicsPipeline_ = VK_NULL_HANDLE;
        }
        if (pipelineLayout_ != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
        if (renderPass_ != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device_, renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
        if (commandPool_ != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device_, commandPool_, nullptr);
            commandPool_ = VK_NULL_HANDLE;
        }
        if (swapchain_ != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
            swapchain_ = VK_NULL_HANDLE;
        }
        if (imageAvailableSemaphore_ != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device_, imageAvailableSemaphore_, nullptr);
        }
        if (renderFinishedSemaphore_ != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device_, renderFinishedSemaphore_, nullptr);
        }
        if (inFlightFence_ != VK_NULL_HANDLE)
        {
            vkDestroyFence(device_, inFlightFence_, nullptr);
        }

        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }

    if (surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }
    if (debugMessenger_ != VK_NULL_HANDLE)
    {
        destroyDebugMessenger(instance_, debugMessenger_);
        debugMessenger_ = VK_NULL_HANDLE;
    }
    if (instance_ != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

bool VulkanRenderer::checkValidationLayerSupport() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const VkLayerProperties &layerProps : availableLayers)
    {
        if (std::strcmp("VK_LAYER_KHRONOS_validation", layerProps.layerName) ==
            0)
            return true;
    }
    return false;
}

std::vector<const char *> VulkanRenderer::getRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount
    );

    if (enableValidationLayers())
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) const
{
    VkPhysicalDeviceProperties prop;
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceProperties(device, &prop);
    vkGetPhysicalDeviceFeatures(device, &features);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data()
    );

    bool queueFound = false;
    for (const VkQueueFamilyProperties &queue : queueFamilies)
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFound = true;
            break;
        }
    }

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, nullptr
    );
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extensionCount, availableExtensions.data()
    );

    bool swapchainFound = false;
    for (const VkExtensionProperties &ext : availableExtensions)
    {
        if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) ==
            0)
        {
            swapchainFound = true;
            break;
        }
    }

    VkBool32 surfaceSupported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        device, 0, surface_, &surfaceSupported
    );

    return prop.deviceType != VK_PHYSICAL_DEVICE_TYPE_OTHER && queueFound &&
           swapchainFound && surfaceSupported;
}

uint32_t VulkanRenderer::findMemoryType(
    uint32_t typeFilter, VkMemoryPropertyFlags properties
) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &formats
) const
{
    for (const VkSurfaceFormatKHR &format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &modes
) const
{
    for (const VkPresentModeKHR &mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR &capabilities
) const
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    return VkExtent2D{width_, height_};
}

uint32_t VulkanRenderer::findQueueFamilyIndex() const
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice_, &queueFamilyCount, nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice_, &queueFamilyCount, queueFamilies.data()
    );

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            return i;
    }
    throw std::runtime_error("Failed to find graphics queue family");
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
