/**
 * @file
 * @brief Vulkan renderer implementation with full pipeline setup.
 */

#pragma once

#include "rendering/IRenderer.h"
#include "scene/SceneData.h"
#include "resource/IResourceManager.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

/**
 * @brief Vulkan renderer backend.
 *
 * Manages full Vulkan pipeline: instance creation, surface, GPU selection,
 * swap chain, render pass, graphics pipeline, descriptor sets, command buffers,
 * and frame rendering with validation layer support.
 */
class VulkanRenderer : public IRenderer
{
    public:
    bool init(GLFWwindow *window) override;
    void render() override;
    void swapBuffers() override;
    void shutdown() override;

    /**
     * @brief Set a callback to check if rendering should stop.
     * @param callback Function returning true if exit is requested.
     */
    void setExitRequestedCallback(std::function<bool()> callback);

    private:
    GLFWwindow *window_ = nullptr;
    std::function<bool()> exitRequested_;

    // Vulkan core objects
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

    // Swap chain
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    std::vector<VkFramebuffer> framebuffers_;
    VkFormat swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent_ = {0, 0};

    // Render pass & pipeline
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;

    // Command buffers
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    // Resources
    BufferAllocation vertexAlloc_ = {};
    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    VkFence inFlightFence_ = VK_NULL_HANDLE;

    // Scene data
    std::vector<Vertex> sceneVertices_;
    uint32_t sceneVertexCount_ = 0;
    bool sceneDirty_ = false;

    // Window dimensions
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    // Instance creation
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void selectGPU();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createVertexBuffer();

    // Shader compilation
    std::vector<char> readFile(const std::string &filename) const;
    VkShaderModule createShaderModule(const std::vector<char> &code) const;

    // Validation
    bool checkValidationLayerSupport() const;
    std::vector<const char *> getRequiredExtensions() const;
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    uint32_t
    findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &formats
    ) const;
    VkPresentModeKHR
    chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &modes) const;
    VkExtent2D
    chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;
    uint32_t findQueueFamilyIndex() const;

    // Cleanup
    void cleanupSwapchain();
    void cleanup();
};
