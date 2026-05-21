/**
 * @file
 * @brief Vulkan renderer implementation with full pipeline setup.
 */

#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "rendering/IRenderer.h"
#include "resource/IResourceManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

struct alignas(16) UBO
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
    void swapBuffers() override;
    void shutdown() override;

    // Frame lifecycle for layer-based rendering
    void beginFrame();
    void renderScene(Scene *scene) override;
    void endFrame();
    bool initImGui() override;
    void newFrame() override;
    void renderImGui() override;
    void shutdownImGui() override;

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

    // Descriptors
    VkDescriptorSetLayout descriptorSetLayout_{};
    VkDescriptorSet descriptorSet_{};
    VkDescriptorPool descriptorPool_{};

    // Command buffers
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    // Resources
    BufferAllocation uniformAlloc_;
    BufferAllocation vertexAlloc_ = {};
    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    VkFence inFlightFence_ = VK_NULL_HANDLE;
    uint32_t allocatedVertexCount_ = 0;

    // ImGui
    ImGuiContext *imguiContext_ = nullptr;
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;
    uint32_t currentImageIndex_ = 0;

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
    void createUniformBuffer();
    void createDescriptorSet();

    void allocateVertexBuffer(uint32_t vertexCount);

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
