#pragma once

#include "core_render/irenderer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_raii.hpp>

class VulkanRenderer : public IRenderer
{
    public:
    bool init(IPlatform *platform) override;
    bool frame() override;
    bool renderImGui() override;

    vk::raii::Device &getDevice();

    private:
    bool initialisePipeline();
    bool beginFrame();
    bool endFrame();
    bool initImGui();
    void shutdownImGui();

    bool createInstance();
    bool setupDebugMessenger();
    bool createSurface();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSynchronizationPrimitives();
    bool createDescriptorPool();

    std::vector<const char *> getRequiredInstanceExtensions();
    bool isDeviceSuitable(vk::PhysicalDevice physicalDevice);
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );
    static uint32_t chooseSwapMinImageCount(
        vk::SurfaceCapabilitiesKHR const &surfaceCapabilities
    );
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        std::vector<vk::SurfaceFormatKHR> const &availableFormats
    );
    static vk::PresentModeKHR chooseSwapPresentMode(
        std::vector<vk::PresentModeKHR> const &availablePresentModes
    );
    vk::Extent2D
    chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);

    uint32_t findQueueFamilyIndex() const;

    IPlatform *platform_ = nullptr;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::raii::PhysicalDevice physicalDevice_ = nullptr;
    vk::raii::Device device_ = nullptr;
    vk::raii::Queue graphicsQueue_ = nullptr;
    vk::raii::SurfaceKHR surface_ = nullptr;
    vk::raii::SwapchainKHR swapChain_ = nullptr;
    std::vector<vk::Image> swapChainImages_;
    vk::SurfaceFormatKHR swapChainSurfaceFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;

    ImGuiContext *imguiContext_ = nullptr;
    vk::DescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    const std::vector<char const *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const vk::DebugUtilsMessageSeverityFlagsEXT debugMessageSeverityFlags_ =
        vk::DebugUtilsMessageSeverityFlagsEXT(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        );
    const vk::DebugUtilsMessageTypeFlagsEXT debugMessageTypeFlags_ =
        vk::DebugUtilsMessageTypeFlagsEXT(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        );
    const std::vector<const char *> requiredDeviceExtension_ = {
        vk::KHRSwapchainExtensionName
    };

    // Unlikely I'll ever want to turn these off
    bool enableValidationLayers_ = true;
};
