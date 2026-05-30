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
    return rasteriser_;
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
        .setPColorAttachmentFormats(&swapChainSurfaceFormat_.format);
}

bool VulkanRenderer::init(IPlatform *platform)
{
    spdlog::debug("Initialising Vulkan renderer");
    platform_ = platform;
    width_ = platform->getConfig().window_width;
    height_ = platform->getConfig().window_height;

    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createSurface();
    createLogicalDevice();

    createSwapChain();
    createImageViews();
    createRenderPass();
    // if (!initImGui())
    //     throw std::runtime_error("ImGui failed to initialise");
    createSynchronizationPrimitives();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createDescriptorPool();

    spdlog::debug("Finished creating descriptor pool");
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
    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size();
         qfpIndex++)
    {
        if ((queueFamilyProperties[qfpIndex].queueFlags &
             vk::QueueFlagBits::eGraphics) &&
            physicalDevice_.getSurfaceSupportKHR(qfpIndex, *surface_))
        {
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0)
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
            .setQueueFamilyIndex(queueIndex)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    // Which features to enable in vulkan
    vk::StructureChain<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {},
            vk::PhysicalDeviceVulkan11Features{}.setShaderDrawParameters(true),
            vk::PhysicalDeviceVulkan13Features{}.setDynamicRendering(true),
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT{}
                .setExtendedDynamicState(true)
        };

    // Enable swapchains
    std::vector<const char *> requiredDeviceExtension = {
        vk::KHRSwapchainExtensionName
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
    graphicsQueue_ = vk::raii::Queue(device_, queueIndex, 0);

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

    std::vector<vk::PresentModeKHR> availablePresentModes =
        physicalDevice_.getSurfacePresentModesKHR(*surface_);
    vk::PresentModeKHR presentMode =
        chooseSwapPresentMode(availablePresentModes);

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

    vk::ImageSubresourceRange imageSubresourceRange =
        vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setLevelCount(1)
            .setLayerCount(1);
    vk::ImageViewCreateInfo imageViewCreateInfo =
        vk::ImageViewCreateInfo{}
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(swapChainSurfaceFormat_.format)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setComponents(
                {vk::ComponentSwizzle::eIdentity,
                 vk::ComponentSwizzle::eIdentity,
                 vk::ComponentSwizzle::eIdentity,
                 vk::ComponentSwizzle::eIdentity}
            )
            .setSubresourceRange(imageSubresourceRange);

    for (vk::Image &image : swapChainImages_)
    {
        imageViewCreateInfo.image = image;
        // emplace back runs the constructor of the type of the vector
        swapChainImageViews_.emplace_back(device_, imageViewCreateInfo);
    }
    spdlog::debug("Finished creating image views");
    return true;
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
    spdlog::debug("Finished creating command pool");
    return true;
}

bool VulkanRenderer::createCommandBuffers()
{
    spdlog::debug("Creating command buffers");
    spdlog::debug("Finished creating command buffers");
    return true;
}

bool VulkanRenderer::createSynchronizationPrimitives()
{
    spdlog::debug("Creating synchronization primitives");
    spdlog::debug("Finished creating synchronization primitives");
    return true;
}

bool VulkanRenderer::beginFrame()
{
    return true;
}

bool VulkanRenderer::endFrame()
{
    return true;
}

bool VulkanRenderer::createDescriptorPool()
{
    return true;
}

bool VulkanRenderer::initImGui()
{
    // imguiContext_ = ImGui::CreateContext();
    // ImGui::SetCurrentContext(imguiContext_);
    //
    // ImGuiIO &io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    // io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    //
    // GLFWwindow *glfwWindow =
    //     static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    // ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);
    //
    // VkDescriptorPoolSize poolSizes[] = {
    //     {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    //     {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    //     {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    //     {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    //     {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    //     {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    //     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    //     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    //     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    //     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    //     {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    // };
    //
    // VkDescriptorPoolCreateInfo poolInfo{};
    // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    // poolInfo.maxSets = 1000 * sizeof(poolSizes) / sizeof(poolSizes[0]);
    // poolInfo.poolSizeCount =
    //     static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
    // poolInfo.pPoolSizes = poolSizes;
    //
    // VkResult result = vkCreateDescriptorPool(
    //     device_, &poolInfo, nullptr, &imguiDescriptorPool_
    // );
    // if (result != VK_SUCCESS)
    // {
    //     spdlog::error(
    //         "Failed to create ImGui descriptor pool: {}",
    //         static_cast<int>(result)
    //     );
    //     imguiDescriptorPool_ = VK_NULL_HANDLE;
    //     return false;
    // }
    //
    // ImGui_ImplVulkan_InitInfo initInfo{};
    // initInfo.Instance = instance_;
    // initInfo.PhysicalDevice = physicalDevice_;
    // initInfo.Device = device_;
    // initInfo.QueueFamily = findQueueFamilyIndex();
    // initInfo.Queue = graphicsQueue_;
    // initInfo.PipelineCache = VK_NULL_HANDLE;
    // initInfo.DescriptorPool = imguiDescriptorPool_;
    // initInfo.Subpass = 0;
    // initInfo.MinImageCount = 2;
    // initInfo.ImageCount = static_cast<uint32_t>(swapchainImages_.size());
    // initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // initInfo.RenderPass = renderPass_;
    //
    // ImGui_ImplVulkan_Init(&initInfo);
    //
    // // Upload font texture
    // ImGui_ImplVulkan_CreateFontsTexture();
    //
    // spdlog::info("ImGui initialised");
    // return true;
}

bool VulkanRenderer::frame()
{
    return true;
}

bool VulkanRenderer::renderImGui()
{
    // ImGui::Render();
    // ImDrawData *drawData = ImGui::GetDrawData();
    // if (drawData->TotalVtxCount > 0)
    // {
    //     ImGui_ImplVulkan_RenderDrawData(
    //         drawData, commandBuffers_[currentImageIndex_]
    //     );
    // }
    //
    return true;
}

void VulkanRenderer::shutdownImGui()
{
    // if (imguiContext_)
    // {
    //     ImGui_ImplVulkan_Shutdown();
    //     ImGui_ImplGlfw_Shutdown();
    //     ImGui::DestroyContext(imguiContext_);
    //     imguiContext_ = nullptr;
    //
    //     if (imguiDescriptorPool_ != VK_NULL_HANDLE)
    //     {
    //         vkDestroyDescriptorPool(device_, imguiDescriptorPool_, nullptr);
    //         imguiDescriptorPool_ = VK_NULL_HANDLE;
    //     }
    //
    //     spdlog::info("ImGui shut down");
    // }
}

uint32_t VulkanRenderer::findQueueFamilyIndex() const
{
    // uint32_t queueFamilyCount = 0;
    // vkGetPhysicalDeviceQueueFamilyProperties(
    //     physicalDevice_, &queueFamilyCount, nullptr
    // );
    // std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    // vkGetPhysicalDeviceQueueFamilyProperties(
    //     physicalDevice_, &queueFamilyCount, queueFamilies.data()
    // );
    //
    // for (uint32_t i = 0; i < queueFamilyCount; i++)
    // {
    //     if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    //         return i;
    // }
    // throw std::runtime_error("Failed to find graphics queue family");
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
