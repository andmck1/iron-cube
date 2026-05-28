#pragma once

#include "core_render/irenderer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <functional>

static constexpr uint64_t kGpuOperationTimeoutNs = 100'000'000;

class VulkanRenderer : public IRenderer
{
    public:
    bool init(IPlatform *platform) override;
    void shutdown() override;
    bool newFrame() override;
    bool renderScene(Scene *scene) override;
    bool renderImGui() override;
    bool drawGeometry(Geometry *geometry) override;

    VkDevice getDevice() const;
    VkRenderPass getRenderPass() const;
    VkDescriptorPool getDescriptorPool() const;

    void bindPipeline(VkPipeline pipeline);
    void bindDescriptorSet(
        VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet
    );

    uint32_t
    findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &formats
    ) const;

    private:
    bool initialisePipeline();
    void swapBuffers();
    bool beginFrame();
    bool endFrame();
    bool initImGui();
    void shutdownImGui();

    void setExitRequestedCallback(std::function<bool()> callback);
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
    void cleanupSwapchain();
    void cleanup();

    bool checkValidationLayerSupport() const;
    std::vector<const char *> getRequiredExtensions() const;
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    VkPresentModeKHR
    chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &modes) const;
    VkExtent2D
    chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    uint32_t findQueueFamilyIndex() const;

    IPlatform *platform_ = nullptr;
    std::function<bool()> exitRequested_;

    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    VkFormat swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent_ = {0, 0};

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;

    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    VkFence inFlightFence_ = VK_NULL_HANDLE;
    ImGuiContext *imguiContext_ = nullptr;
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;
    uint32_t currentImageIndex_ = 0;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
};
