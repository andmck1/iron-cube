#include "core_render/vulkan/renderer.h"
#include "core_platform/linux/platform.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <string>

vk::raii::Device &VulkanRenderer::getDevice()
{
    return device_;
}

vk::raii::PhysicalDevice &VulkanRenderer::getPhysicalDevice()
{
    return physicalDevice_;
}

vk::raii::CommandBuffer &VulkanRenderer::getCommandBuffer()
{
    return commandBuffers_[currentFrameIndex_];
}

vk::Extent2D &VulkanRenderer::getSwapChainExtent()
{
    return swapChainExtent_;
}

vk::Viewport VulkanRenderer::getViewport()
{
    // TODO - generalise a bit
    return {
        0.0f,
        0.0f,
        static_cast<float>(swapChainExtent_.width),
        static_cast<float>(swapChainExtent_.height),
        0.0f,
        1.0f
    };
}

vk::PipelineInputAssemblyStateCreateInfo VulkanRenderer::getInputAssembly()
{
    return inputAssembly_;
}

vk::PipelineRasterizationStateCreateInfo VulkanRenderer::getRasteriser()
{
    vk::PipelineRasterizationStateCreateInfo result = rasteriser_;
    result.setPolygonMode(
        wireframeMode_ ? vk::PolygonMode::eLine : vk::PolygonMode::eFill
    );
    return result;
}

bool VulkanRenderer::isWireframeMode() const
{
    return wireframeMode_;
}

void VulkanRenderer::setWireframeMode(bool enabled)
{
    wireframeMode_ = enabled;
}

vk::PipelineMultisampleStateCreateInfo VulkanRenderer::getMultisampling()
{
    return multisampling_;
}

vk::PipelineColorBlendAttachmentState VulkanRenderer::getColourBlendAttachment()
{
    return colourBlendAttachment_;
}

vk::PipelineRenderingCreateInfo
VulkanRenderer::getPipelineRendereringCreateInfo()
{
    return vk::PipelineRenderingCreateInfo{}
        .setViewMask(0)
        .setColorAttachmentCount(1)
        .setPColorAttachmentFormats(&swapChainSurfaceFormat_.format)
        .setDepthAttachmentFormat(findDepthFormat());
}

uint32_t &VulkanRenderer::getCurrentFrameIndex()
{
    return currentFrameIndex_;
}

bool VulkanRenderer::init(IPlatform *platform)
{
    spdlog::debug("Initialising Vulkan renderer");
    platform_ = platform;
    width_ = platform->getConfig().window_width;
    height_ = platform->getConfig().window_height;

    // TODO - proper error checking?
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createSurface();
    createLogicalDevice();

    createSwapChain();
    createImageViews();
    createRenderPass();
    if (!initImGui())
        throw std::runtime_error("ImGui failed to initialise");
    createSyncObjects();
    createFramebuffers();
    createCommandPool();
    createDepthResources();
    createCommandBuffers();

    return true;
}

bool VulkanRenderer::createInstance()
{
    spdlog::debug("Creating Vulkan instance");
    constexpr vk::ApplicationInfo appInfo =
        vk::ApplicationInfo{}
            .setPApplicationName("Iron Cube")
            .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
            .setPEngineName("Iron Engine")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(vk::ApiVersion14);

    // Check for validation layers
    // Vulkan documentation
    std::vector<char const *> requiredLayers;
    if (enableValidationLayers_)
    {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }
    std::vector<vk::LayerProperties> layerProperties =
        context_.enumerateInstanceLayerProperties();
    if (std::ranges::any_of(
            requiredLayers,
            [&layerProperties](const char *&requiredLayer)
            {
                return std::ranges::none_of(
                    layerProperties,
                    [requiredLayer](vk::LayerProperties const &layerProperty)
                    {
                        return strcmp(layerProperty.layerName, requiredLayer) ==
                               0;
                    }
                );
            }
        ))
    {
        throw std::runtime_error(
            "One or more required layers are not supported!"
        );
    }

    // As Vulkan is platform agnostic we need to add GLFW extensions manually
    // Vulkan documentation
    std::vector<const char *> requiredExtensions =
        getRequiredInstanceExtensions();
    std::vector<vk::ExtensionProperties> extensionProperties =
        context_.enumerateInstanceExtensionProperties();
    auto unsupportedPropertyIt = std::ranges::find_if(
        requiredExtensions,
        [&extensionProperties](const char *&requiredExtension)
        {
            return std::ranges::none_of(
                extensionProperties,
                [requiredExtension](
                    vk::ExtensionProperties const &extensionProperty
                )
                {
                    return strcmp(
                               extensionProperty.extensionName,
                               requiredExtension
                           ) == 0;
                }
            );
        }
    );
    if (unsupportedPropertyIt != requiredExtensions.end())
    {
        throw std::runtime_error(
            "Required extension not supported: " +
            std::string(*unsupportedPropertyIt)
        );
    }

    vk::InstanceCreateInfo createInfo =
        vk::InstanceCreateInfo{}
            .setPApplicationInfo(&appInfo)
            .setEnabledLayerCount(static_cast<uint32_t>(requiredLayers.size()))
            .setPpEnabledLayerNames(requiredLayers.data())
            .setEnabledExtensionCount(
                static_cast<uint32_t>(requiredExtensions.size())
            )
            .setPpEnabledExtensionNames(requiredExtensions.data());
    try
    {
        instance_ = vk::raii::Instance(context_, createInfo);
    }
    catch (const vk::SystemError &err)
    {
        spdlog::error("Vulkan error while creating instance: {}", err.what());
        return false;
    }
    catch (const std::exception &err)
    {
        spdlog::error("Error while creating instance: {}", err.what());
        return false;
    }

    spdlog::debug("Finished creating vulkan instance");
    return true;
}

bool VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers_)
        return true;
    spdlog::debug("Setting up debug messenger");

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT =
        vk::DebugUtilsMessengerCreateInfoEXT{}
            .setMessageSeverity(debugMessageSeverityFlags_)
            .setMessageType(debugMessageTypeFlags_)
            .setPfnUserCallback(&debugCallback);
    debugMessenger_ = instance_.createDebugUtilsMessengerEXT(
        debugUtilsMessengerCreateInfoEXT
    );

    spdlog::debug("Finished setting up debug messenger");
    return true;
}

bool VulkanRenderer::createSurface()
{
    spdlog::debug("Creating surface");

    // TODO - not generalised to any platform. Is likely I should pass the
    // window to this function
    GLFWwindow *glfwWindow =
        static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*instance_, glfwWindow, nullptr, &surface) != 0)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    surface_ = vk::raii::SurfaceKHR(instance_, surface);

    spdlog::debug("Finished creating surface");
    return true;
}

bool VulkanRenderer::pickPhysicalDevice()
{
    spdlog::debug("Selecting physical device");

    std::vector<vk::raii::PhysicalDevice> physicalDevices =
        instance_.enumeratePhysicalDevices();

    if (physicalDevices.empty())
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    auto const physicalDeviceIterator = std::ranges::find_if(
        physicalDevices, [&](vk::PhysicalDevice const &physicalDevice)
        { return isDeviceSuitable(physicalDevice); }
    );
    if (physicalDeviceIterator == physicalDevices.end())
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    physicalDevice_ = *physicalDeviceIterator;

    spdlog::debug("Finished selecting physical device");
    return true;
}

bool VulkanRenderer::createLogicalDevice()
{
    spdlog::debug("Creating logical device");

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice_.getQueueFamilyProperties();

    // get the first index into queueFamilyProperties which supports both
    // graphics and present
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size();
         qfpIndex++)
    {
        if ((queueFamilyProperties[qfpIndex].queueFlags &
             vk::QueueFlagBits::eGraphics) &&
            physicalDevice_.getSurfaceSupportKHR(qfpIndex, *surface_))
        {
            queueIndex_ = qfpIndex;
            break;
        }
    }
    if (queueIndex_ == ~0)
    {
        throw std::runtime_error(
            "Could not find a queue for graphics and present"
        );
    }

    // If we had multiple queues we can control their priority
    // Necessary even if only one queue
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo =
        vk::DeviceQueueCreateInfo{}
            .setQueueFamilyIndex(queueIndex_)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    // Which features to enable in vulkan
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
        vk::PhysicalDeviceMeshShaderFeaturesEXT>
        featureChain = {
            vk::PhysicalDeviceFeatures2{}.setFeatures(
                vk::PhysicalDeviceFeatures{}
                    .setFillModeNonSolid(true)
                    .setPipelineStatisticsQuery(true)
                    .setVertexPipelineStoresAndAtomics(true)
            ),
            vk::PhysicalDeviceVulkan11Features{}.setShaderDrawParameters(true),
            vk::PhysicalDeviceVulkan13Features{}
                .setDynamicRendering(true)
                .setSynchronization2(true),
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{}
                .setExtendedDynamicState(true),
            vk::PhysicalDeviceMeshShaderFeaturesEXT{}
                .setMeshShader(true)
                .setTaskShader(true)
                .setMeshShaderQueries(true)
        };

    // Enable swapchains
    std::vector<const char *> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName, vk::EXTMeshShaderExtensionName
    };

    vk::DeviceCreateInfo deviceCreateInfo =
        vk::DeviceCreateInfo{}
            .setPNext(&featureChain.get<vk::PhysicalDeviceFeatures2>())
            .setQueueCreateInfoCount(1)
            .setPQueueCreateInfos(
                reinterpret_cast<const vk::DeviceQueueCreateInfo *>(
                    &deviceQueueCreateInfo
                )
            )
            .setEnabledExtensionCount(
                static_cast<uint32_t>(requiredDeviceExtension.size())
            )
            .setPpEnabledExtensionNames(requiredDeviceExtension.data());

    device_ = vk::raii::Device(physicalDevice_, deviceCreateInfo);
    queue_ = vk::raii::Queue(device_, queueIndex_, 0);

    spdlog::debug("Finished creating logical device");
    return true;
}

bool VulkanRenderer::createSwapChain()
{
    // ty vulkan docs for swapchain stuff
    spdlog::debug("Creating swap chain");

    vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        physicalDevice_.getSurfaceCapabilitiesKHR(*surface_);
    swapChainExtent_ = chooseSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> availableFormats =
        physicalDevice_.getSurfaceFormatsKHR(*surface_);
    swapChainSurfaceFormat_ = chooseSwapSurfaceFormat(availableFormats);

    // Forcing off Vsync to uncap fps
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eImmediate;
    // std::vector<vk::PresentModeKHR> availablePresentModes =
    //     physicalDevice_.getSurfacePresentModesKHR(*surface_);
    // vk::PresentModeKHR presentMode =
    //     chooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo =
        vk::SwapchainCreateInfoKHR{}
            .setSurface(*surface_)
            .setMinImageCount(minImageCount)
            .setImageFormat(swapChainSurfaceFormat_.format)
            .setImageColorSpace(swapChainSurfaceFormat_.colorSpace)
            .setImageExtent(swapChainExtent_)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setPreTransform(surfaceCapabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(true);

    swapChain_ = vk::raii::SwapchainKHR(device_, swapChainCreateInfo);
    swapChainImages_ = swapChain_.getImages();

    spdlog::debug("Finished creating swap chain");
    return true;
}

bool VulkanRenderer::createImageViews()
{
    spdlog::debug("Creating image views");

    if (!swapChainImageViews_.empty())
    {
        throw std::runtime_error("Failed to create image views");
    }

    swapChainImageViews_.reserve(swapChainImages_.size());
    for (auto &image : swapChainImages_)
    {
        swapChainImageViews_.emplace_back(createImageView(
            image, swapChainSurfaceFormat_.format,
            vk::ImageAspectFlagBits::eColor
        ));
    }

    spdlog::debug("Finished creating image views");
    return true;
}

vk::raii::ImageView VulkanRenderer::createImageView(
    vk::Image const &image, vk::Format format, vk::ImageAspectFlags aspectFlags
)
{
    vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo{}
                                           .setImage(image)
                                           .setViewType(vk::ImageViewType::e2D)
                                           .setFormat(format)
                                           .setSubresourceRange(
                                               vk::ImageSubresourceRange{}
                                                   .setAspectMask(aspectFlags)
                                                   .setBaseMipLevel(0)
                                                   .setLevelCount(1)
                                                   .setBaseArrayLayer(0)
                                                   .setLayerCount(1)
                                           );
    return vk::raii::ImageView(device_, viewInfo);
}

bool VulkanRenderer::createRenderPass()
{
    spdlog::debug("Creating render pass");
    spdlog::debug("Finished creating render pass");
    return true;
}

bool VulkanRenderer::createFramebuffers()
{
    spdlog::debug("Creating framebuffers");
    spdlog::debug("Finished creating framebuffers");
    return true;
}

bool VulkanRenderer::createCommandPool()
{
    spdlog::debug("Creating command pool");

    vk::CommandPoolCreateInfo poolInfo =
        vk::CommandPoolCreateInfo{}
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(queueIndex_);

    commandPool_ = vk::raii::CommandPool(device_, poolInfo);

    spdlog::debug("Finished creating command pool");
    return true;
}

bool VulkanRenderer::createCommandBuffers()
{
    spdlog::debug("Creating command buffers");

    commandBuffers_.clear();

    vk::CommandBufferAllocateInfo allocInfo =
        vk::CommandBufferAllocateInfo{}
            .setCommandPool(commandPool_)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(MAX_FRAMES_IN_FLIGHT);

    commandBuffers_ = vk::raii::CommandBuffers(device_, allocInfo);

    spdlog::debug("Finished creating command buffer");
    return true;
}

bool VulkanRenderer::createDepthResources()
{
    spdlog::debug("Creating depth resources");

    vk::Format depthFormat = findDepthFormat();

    std::tie(depthImage_, depthImageMemory_) = createImage(
        swapChainExtent_.width, swapChainExtent_.height, depthFormat,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    depthImageView_ = createImageView(
        depthImage_, depthFormat, vk::ImageAspectFlagBits::eDepth
    );

    spdlog::debug("Finished creating depth resources");

    return true;
}

bool VulkanRenderer::createSyncObjects()
{
    spdlog::debug("Creating sync objects");
    assert(
        presentCompleteSemaphores_.empty() &&
        renderFinishedSemaphores_.empty() && inFlightFences_.empty()
    );

    // renderFinishedSemaphores_ is indexed by swapchain image index because
    // the same image can be acquired by different frames
    for (size_t i = 0; i < swapChainImages_.size(); i++)
    {
        renderFinishedSemaphores_.emplace_back(
            device_, vk::SemaphoreCreateInfo()
        );
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        presentCompleteSemaphores_.emplace_back(
            device_, vk::SemaphoreCreateInfo()
        );
        inFlightFences_.emplace_back(
            device_,
            vk::FenceCreateInfo{}.setFlags(vk::FenceCreateFlagBits::eSignaled)
        );
    }

    spdlog::debug("Finished creating sync objects");
    return true;
}

bool VulkanRenderer::beginFrame()
{
    // Wait for previous frame to finish
    vk::Result fenceResult = device_.waitForFences(
        *inFlightFences_[currentFrameIndex_], vk::True, UINT64_MAX
    );
    if (fenceResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence!");
    }
    device_.resetFences(*inFlightFences_[currentFrameIndex_]);

    // Get an image from the framebuffer now the previous frame is finished
    auto [_, imageIndex] = swapChain_.acquireNextImage(
        UINT64_MAX, *presentCompleteSemaphores_[currentFrameIndex_], nullptr
    );
    currentImageIndex_ = imageIndex;

    // TODO - move??
    ImGui::NewFrame();

    // Begin recording commands
    vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();
    commandBuffer.begin({});
    transitionImageLayout(
        swapChainImages_[imageIndex], vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal, {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );
    transitionImageLayout(
        *depthImage_, vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests |
            vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests |
            vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth
    );

    return true;
}

bool VulkanRenderer::beginRender()
{
    vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();
    vk::ClearValue clearColour = vk::ClearColorValue(0.25f, 0.25f, 0.25f, 1.f);
    vk::RenderingAttachmentInfo attachmentInfo =
        vk::RenderingAttachmentInfo{}
            .setImageView(swapChainImageViews_[currentImageIndex_])
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(clearColour);

    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderingAttachmentInfo depthAttachmentInfo =
        vk::RenderingAttachmentInfo{}
            .setImageView(depthImageView_)
            .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setClearValue(clearDepth);

    vk::RenderingInfo renderingInfo =
        vk::RenderingInfo{}
            .setRenderArea({{0, 0}, swapChainExtent_})
            .setLayerCount(1)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&attachmentInfo)
            .setPDepthAttachment(&depthAttachmentInfo);

    commandBuffer.beginRendering(renderingInfo);
    return true;
}

bool VulkanRenderer::endRender()
{
    vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();
    commandBuffer.endRendering();
    return true;
}

bool VulkanRenderer::drawFrame()
{
    vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();

    transitionImageLayout(
        swapChainImages_[currentImageIndex_],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    commandBuffer.end();

    // Execute draw commands
    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo =
        vk::SubmitInfo{}
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(
                &*presentCompleteSemaphores_[currentFrameIndex_]
            )
            .setPWaitDstStageMask(&waitDestinationStageMask)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*commandBuffer)
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(
                &*renderFinishedSemaphores_[currentImageIndex_]
            );
    queue_.submit(submitInfo, *inFlightFences_[currentFrameIndex_]);

    return true;
}

bool VulkanRenderer::present()
{
    const vk::PresentInfoKHR presentInfoKHR =
        vk::PresentInfoKHR{}
            .setWaitSemaphoreCount(1)
            .setPWaitSemaphores(&*renderFinishedSemaphores_[currentImageIndex_])
            .setSwapchainCount(1)
            .setPSwapchains(&*swapChain_)
            .setPImageIndices(&currentImageIndex_)
            .setPResults(nullptr);
    auto _ = queue_.presentKHR(presentInfoKHR);
    currentFrameIndex_ = (currentFrameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

void VulkanRenderer::waitAllFramesComplete()
{
    spdlog::debug("Waiting for all frames to complete");
    device_.waitIdle();
    spdlog::debug("All frames complete");
}

bool VulkanRenderer::shutdown()
{
    spdlog::debug("Shutting down the vulkan renderer");

    device_.waitIdle();
    shutdownImGui();

    spdlog::debug("Vulkan renderer shutdown");
    return true;
}

// TODO - move this to a separate VulkanDebug class?
bool VulkanRenderer::initImGui()
{
    spdlog::debug("Initialising ImGui");
    imguiContext_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext_);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    GLFWwindow *glfwWindow =
        static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

    vk::DescriptorPoolSize poolSizes[] = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000}
    };

    vk::DescriptorPoolCreateInfo poolInfo =
        vk::DescriptorPoolCreateInfo{}
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(
                static_cast<uint32_t>(
                    1000 * sizeof(poolSizes) / sizeof(poolSizes[0])
                )
            )
            .setPoolSizeCount(
                static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]))
            )
            .setPPoolSizes(poolSizes);

    try
    {
        imguiDescriptorPool_ = vk::raii::DescriptorPool(device_, poolInfo);
    }
    catch (const vk::SystemError &err)
    {
        spdlog::error("Failed to create ImGui descriptor pool: {}", err.what());
        return false;
    }

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = *instance_;
    initInfo.PhysicalDevice = *physicalDevice_;
    initInfo.Device = *device_;
    initInfo.QueueFamily = queueIndex_;
    initInfo.Queue = *queue_;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = *imguiDescriptorPool_;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(swapChainImages_.size());
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass = VK_NULL_HANDLE;
    initInfo.UseDynamicRendering = true;

    initInfo.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    initInfo.PipelineRenderingCreateInfo.pNext = nullptr;
    initInfo.PipelineRenderingCreateInfo.viewMask = 0;
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        reinterpret_cast<VkFormat *>(&swapChainSurfaceFormat_.format);
    initInfo.PipelineRenderingCreateInfo.depthAttachmentFormat =
        VK_FORMAT_D32_SFLOAT;
    initInfo.PipelineRenderingCreateInfo.stencilAttachmentFormat =
        VK_FORMAT_UNDEFINED;

    ImGui_ImplVulkan_Init(&initInfo);

    // Upload font texture
    ImGui_ImplVulkan_CreateFontsTexture();

    spdlog::debug("ImGui initialised");
    return true;
}

bool VulkanRenderer::renderImGui()
{
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    if (drawData->TotalVtxCount > 0)
    {
        vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();
        ImGui_ImplVulkan_RenderDrawData(drawData, *commandBuffer);
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

        imguiDescriptorPool_ = nullptr;

        spdlog::debug("ImGui shut down");
    }
}

uint32_t VulkanRenderer::findQueueFamilyIndex() const
{
    std::vector<vk::QueueFamilyProperties> queueFamilies =
        physicalDevice_.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
            return i;
    }
    throw std::runtime_error("Failed to find graphics queue family");
}

std::vector<const char *> VulkanRenderer::getRequiredInstanceExtensions()
{
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers_)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

bool VulkanRenderer::isDeviceSuitable(vk::PhysicalDevice physicalDevice)
{
    // TODO - add check for mesh shader support?

    // API support
    bool supportsVulkan1_3 =
        physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
    std::vector<vk::QueueFamilyProperties> queueFamilies =
        physicalDevice.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(
        queueFamilies,
        [](vk::QueueFamilyProperties const &queueFamilyProperties)
        {
            return !!(
                queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics
            );
        }
    );
    std::vector<const char *> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName
    };

    // Queue families support graphics operations
    std::vector<vk::ExtensionProperties> availableDeviceExtensions =
        physicalDevice.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtension,
        [&availableDeviceExtensions](const char *&requiredDeviceExtension)
        {
            return std::ranges::any_of(
                availableDeviceExtensions,
                [requiredDeviceExtension](
                    vk::ExtensionProperties const &availableDeviceExtension
                )
                {
                    return strcmp(
                               availableDeviceExtension.extensionName,
                               requiredDeviceExtension
                           ) == 0;
                }
            );
        }
    );

    // Feature support
    auto features = physicalDevice.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .dynamicRendering &&
        features
            .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
            .extendedDynamicState;

    return supportsVulkan1_3 && supportsGraphics &&
           supportsAllRequiredExtensions && supportsRequiredFeatures;
}

uint32_t VulkanRenderer::chooseSwapMinImageCount(
    vk::SurfaceCapabilitiesKHR const &surfaceCapabilities
)
{
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if ((0 < surfaceCapabilities.maxImageCount) &&
        (surfaceCapabilities.maxImageCount < minImageCount))
    {
        minImageCount = surfaceCapabilities.maxImageCount;
    }
    return minImageCount;
}

vk::SurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &availableFormats
)
{
    assert(!availableFormats.empty());
    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb &&
                   format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        }
    );
    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}

vk::PresentModeKHR VulkanRenderer::chooseSwapPresentMode(
    std::vector<vk::PresentModeKHR> const &availablePresentModes
)
{
    assert(
        std::ranges::any_of(
            availablePresentModes, [](auto presentMode)
            { return presentMode == vk::PresentModeKHR::eFifo; }
        )
    );
    return std::ranges::any_of(
               availablePresentModes, [](const vk::PresentModeKHR value)
               { return vk::PresentModeKHR::eMailbox == value; }
           )
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

vk::Extent2D
VulkanRenderer::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
{
    GLFWwindow *glfwWindow =
        static_cast<PlatformLinux *>(platform_)->getGLFWWindow();

    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);

    return {
        std::clamp<uint32_t>(
            width, capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        ),
    };
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanRenderer::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData
)
{
    spdlog::error(
        "validation layer: type {} msg: {}", vk::to_string(type),
        pCallbackData->pMessage
    );

    return vk::False;
}

void VulkanRenderer::transitionImageLayout(
    vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
    vk::ImageAspectFlags imageAspectFlags
)
{
    vk::raii::CommandBuffer &commandBuffer = getCommandBuffer();
    vk::ImageMemoryBarrier2 barrier =
        vk::ImageMemoryBarrier2{}
            .setSrcStageMask(srcStageMask)
            .setSrcAccessMask(srcAccessMask)
            .setDstStageMask(dstStageMask)
            .setDstAccessMask(dstAccessMask)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(image)
            .setSubresourceRange(
                vk::ImageSubresourceRange{}
                    .setAspectMask(imageAspectFlags)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)
            );
    vk::DependencyInfo dependencyInfo = vk::DependencyInfo{}
                                            .setDependencyFlags({})
                                            .setImageMemoryBarrierCount(1)
                                            .setPImageMemoryBarriers(&barrier);
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

uint32_t VulkanRenderer::findMemoryType(
    uint32_t typeFilter, vk::MemoryPropertyFlags properties
)
{
    vk::PhysicalDeviceMemoryProperties memProperties =
        physicalDevice_.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory>
VulkanRenderer::createBuffer(
    vk::DeviceSize size, vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties
)
{
    vk::BufferCreateInfo bufferInfo =
        vk::BufferCreateInfo{}.setSize(size).setUsage(usage).setSharingMode(
            vk::SharingMode::eExclusive
        );
    vk::raii::Buffer buffer = vk::raii::Buffer(device_, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(
                findMemoryType(memRequirements.memoryTypeBits, properties)
            );
    vk::raii::DeviceMemory bufferMemory =
        vk::raii::DeviceMemory(device_, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
    return {std::move(buffer), std::move(bufferMemory)};
}

bool VulkanRenderer::copyBuffer(
    vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer,
    vk::DeviceSize size
)
{
    vk::CommandBufferAllocateInfo allocInfo =
        vk::CommandBufferAllocateInfo{}
            .setCommandPool(commandPool_)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);
    vk::raii::CommandBuffer commandCopyBuffer =
        std::move(device_.allocateCommandBuffers(allocInfo).front());
    commandCopyBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandCopyBuffer.copyBuffer(
        *srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size)
    );
    commandCopyBuffer.end();
    queue_.submit(
        vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(
            &*commandCopyBuffer
        ),
        nullptr
    );
    queue_.waitIdle();
    return true;
}

vk::Format VulkanRenderer::findSupportedFormat(
    const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
    vk::FormatFeatureFlags features
)
{
    for (const auto format : candidates)
    {
        vk::FormatProperties props =
            physicalDevice_.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal &&
            (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

vk::Format VulkanRenderer::findDepthFormat()
{
    return findSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
         vk::Format::eD24UnormS8Uint},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> VulkanRenderer::createImage(
    uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties
)
{
    vk::ImageCreateInfo imageInfo =
        vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(format)
            .setExtent({width, height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(tiling)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive);

    vk::raii::Image image = vk::raii::Image(device_, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo =
        vk::MemoryAllocateInfo{}
            .setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(
                findMemoryType(memRequirements.memoryTypeBits, properties)
            );
    vk::raii::DeviceMemory imageMemory =
        vk::raii::DeviceMemory(device_, allocInfo);
    image.bindMemory(imageMemory, 0);

    return {std::move(image), std::move(imageMemory)};
}
