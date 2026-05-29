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
    bool createSwapchain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSynchronizationPrimitives();
    bool createDescriptorPool();

    std::vector<const char *> getRequiredInstanceExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData
    );

    uint32_t findQueueFamilyIndex() const;

    IPlatform *platform_ = nullptr;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::raii::PhysicalDevice physicalDevice_ = nullptr;

    ImGuiContext *imguiContext_ = nullptr;
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    const std::vector<char const *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const VkDebugUtilsMessageSeverityFlagsEXT debugMessageSeverityFlags_ =
        VkDebugUtilsMessageSeverityFlagsEXT(
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        );
    const VkDebugUtilsMessageTypeFlagsEXT debugMessageTypeFlags_ =
        VkDebugUtilsMessageTypeFlagsEXT(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        );

    // Unlikely I'll ever want to turn these off
    bool enableValidationLayers_ = true;
};
