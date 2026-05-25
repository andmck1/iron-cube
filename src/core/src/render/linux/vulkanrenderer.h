#pragma once

#include "buffers/ubuffer.h"
#include "material/material.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "render/irenderer.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

static constexpr uint64_t kGpuOperationTimeoutNs = 100'000'000;

struct BufferAllocation
{
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDeviceSize size{0};
};

class VulkanRenderer : public IRenderer
{
    public:
    bool init(IPlatform *platform) override;
    void swapBuffers() override;
    void shutdown() override;

    bool beginFrame();
    bool newFrame() override;
    bool renderScene(Scene *scene) override;
    bool endFrame();
    bool initImGui() override;
    bool renderImGui() override;
    void shutdownImGui() override;

    void setExitRequestedCallback(std::function<bool()> callback);

    void setUniformModelMatrix(glm::mat4 model) override;
    bool drawGeometry(Geometry *geometry) override;
    void setDeltaTime(float deltaSeconds) override;

    private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void selectGPU();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline(const ShaderPipeline &pipeline);
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createUniformBuffer();
    void createDescriptorSet();
    void cleanupSwapchain();
    void cleanup();

    BufferAllocation allocateBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties
    );
    void *mapMemory(BufferAllocation &alloc, VkDeviceSize size);
    void unmapMemory(BufferAllocation &alloc);
    void freeBuffer(BufferAllocation &alloc);
    bool allocateVertexBuffer(uint32_t vertexCount);

    std::vector<char> readFile(const std::string &filename) const;
    VkShaderModule createShaderModule(const std::vector<char> &code) const;
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
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout_{};
    VkDescriptorSet descriptorSet_{};
    VkDescriptorPool descriptorPool_{};

    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    BufferAllocation uniformAlloc_;
    BufferAllocation vertexAlloc_ = {};
    VkSemaphore imageAvailableSemaphore_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore_ = VK_NULL_HANDLE;
    VkFence inFlightFence_ = VK_NULL_HANDLE;
    uint32_t allocatedVertexCount_ = 0;

    ImGuiContext *imguiContext_ = nullptr;
    VkDescriptorPool imguiDescriptorPool_ = VK_NULL_HANDLE;
    uint32_t currentImageIndex_ = 0;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    glm::mat4 currentModelMatrix_ = glm::mat4(1.0f);
    bool uniformDirty_ = false;
    float deltaTime_ = 1.0f / 60.0f;

    ShaderPipeline currentPipeline_;
};
