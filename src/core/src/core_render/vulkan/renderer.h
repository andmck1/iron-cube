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
    bool renderImGui() override;
    bool beginFrame() override;
    bool drawFrame() override;
    bool present() override;
    bool shutdown() override;

    bool draw(Geometry *geometry) override;

    vk::raii::Device &getDevice();
    vk::raii::PhysicalDevice &getPhysicalDevice();
    vk::raii::CommandBuffer &getCommandBuffer();
    vk::raii::DescriptorPool &getDescriptorPool();
    vk::Extent2D &getSwapChainExtent();
    vk::Viewport getViewport();
    vk::PipelineInputAssemblyStateCreateInfo getInputAssembly();
    vk::PipelineRasterizationStateCreateInfo getRasteriser();
    vk::PipelineMultisampleStateCreateInfo getMultisampling();
    vk::PipelineColorBlendAttachmentState getColourBlendAttachment();
    vk::PipelineRenderingCreateInfo getPipelineRendereringCreateInfo();
    uint32_t &getCurrentFrameIndex();
    uint32_t
    findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
        vk::DeviceSize size, vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );
    bool copyBuffer(
        vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer,
        vk::DeviceSize size
    );

    // TODO - incorporate this
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    private:
    bool initialisePipeline();
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
    bool createSyncObjects();
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
    void recordCommandBuffer(uint32_t imageIndex);
    void transition_image_layout(
        uint32_t imageIndex, vk::ImageLayout old_layout,
        vk::ImageLayout new_layout, vk::AccessFlags2 src_access_mask,
        vk::AccessFlags2 dst_access_mask,
        vk::PipelineStageFlags2 src_stage_mask,
        vk::PipelineStageFlags2 dst_stage_mask
    );

    uint32_t findQueueFamilyIndex() const;

    IPlatform *platform_ = nullptr;

    vk::raii::Context context_;
    vk::raii::Instance instance_ = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger_ = nullptr;
    vk::raii::PhysicalDevice physicalDevice_ = nullptr;
    vk::raii::Device device_ = nullptr;
    uint32_t queueIndex_ = ~0;
    vk::raii::Queue queue_ = nullptr;
    vk::raii::SurfaceKHR surface_ = nullptr;
    vk::raii::SwapchainKHR swapChain_ = nullptr;
    std::vector<vk::Image> swapChainImages_;
    vk::SurfaceFormatKHR swapChainSurfaceFormat_;
    vk::Extent2D swapChainExtent_;
    std::vector<vk::raii::ImageView> swapChainImageViews_;
    vk::raii::CommandPool commandPool_ = nullptr;
    std::vector<vk::raii::CommandBuffer> commandBuffers_;
    vk::raii::DescriptorPool descriptorPool_ = nullptr;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores_;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::raii::Fence> inFlightFences_;
    uint32_t currentFrameIndex_ = 0;

    ImGuiContext *imguiContext_ = nullptr;
    vk::raii::DescriptorPool imguiDescriptorPool_ = nullptr;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t currentImageIndex_ = 0;

    // TODO - consider moving this somewhere else
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly_ =
        vk::PipelineInputAssemblyStateCreateInfo{}.setTopology(
            vk::PrimitiveTopology::eTriangleList
        );
    vk::PipelineRasterizationStateCreateInfo rasteriser_ =
        vk::PipelineRasterizationStateCreateInfo{}
            .setDepthClampEnable(vk::False)
            .setRasterizerDiscardEnable(vk::False)
            // For wireframe set to eLine
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(vk::False)
            .setLineWidth(1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampling_ =
        vk::PipelineMultisampleStateCreateInfo{}
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);
    vk::PipelineColorBlendAttachmentState colourBlendAttachment_ =
        vk::PipelineColorBlendAttachmentState{}
            .setBlendEnable(vk::True)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            );

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
