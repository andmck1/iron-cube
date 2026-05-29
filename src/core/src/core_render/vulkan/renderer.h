#pragma once

#include "core_render/irenderer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

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

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void selectGPU();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSynchronizationPrimitives();
    void createDescriptorPool();

    uint32_t findQueueFamilyIndex() const;

    IPlatform *platform_ = nullptr;

    VkDevice device_ = VK_NULL_HANDLE;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;
    VkQueue graphicsQueue_;

    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    VkFence inFlightFence_ = VK_NULL_HANDLE;
    ImGuiContext *imguiContext_ = nullptr;
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;
    uint32_t currentImageIndex_ = 0;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
};
