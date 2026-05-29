#include "core_render/vulkan/renderer.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <string>

bool VulkanRenderer::init(IPlatform *platform)
{
    spdlog::debug("Initialising Vulkan renderer");
    platform_ = platform;
    width_ = platform->getConfig().window_width;
    height_ = platform->getConfig().window_height;

    createInstance();
    setupDebugMessenger();
    createSurface();
    createSwapchain();
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
    constexpr VkApplicationInfo appInfo{
        .pApplicationName = "Iron Cube",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Iron Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

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

    VkInstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount =
            static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };
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

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = debugMessageSeverityFlags_,
        .messageType = debugMessageTypeFlags_,
        .pfnUserCallback = &debugCallback
    };
    debugMessenger_ = instance_.createDebugUtilsMessengerEXT(
        debugUtilsMessengerCreateInfoEXT
    );

    spdlog::debug("Finished setting up debug messenger");
    return true;
}

bool VulkanRenderer::createSurface()
{
    spdlog::debug("Creating surface");
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

    for (vk::raii::PhysicalDevice physicalDevice : physicalDevices)
    {
        bool supportsVulkan1_3 =
            physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
        std::vector<vk::QueueFamilyProperties> queueFamilies =
            physicalDevice.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(
            queueFamilies,
            [](vk::QueueFamilyProperties const &queueFamilyProperties)
            {
                return !!(
                    queueFamilyProperties.queueFlags &
                    vk::QueueFlagBits::eGraphics
                );
            }
        );
        std::vector<const char *> requiredDeviceExtension = {
            vk::KHRSwapchainExtensionName
        };

        std::vector<vk::ExtensionProperties> availableDeviceExtensions =
            physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            requiredDeviceExtension,
            [&availableDeviceExtensions](const char *&requiredDeviceExtension)
            {
                return std::ranges::any_of(
                    availableDeviceExtensions,
                    [requiredDeviceExtension](
                        VkExtensionProperties const &availableDeviceExtension
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

        auto features = physicalDevice.template getFeatures2<
            vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan13Features>()
                .dynamicRendering &&
            features
                .template get<
                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
                .extendedDynamicState;
    }

    spdlog::debug("Finished selecting physical device");
    return true;
}

bool VulkanRenderer::createSwapchain()
{
    spdlog::debug("Creating swap chain");
    spdlog::debug("Finished creating swap chain");
    return true;
}

bool VulkanRenderer::createImageViews()
{
    spdlog::debug("Creating image views");
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

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData
)
{
    spdlog::error(
        "validation layer: type {} msg: {}", std::to_string(type),
        pCallbackData->pMessage
    );

    return vk::False;
}
