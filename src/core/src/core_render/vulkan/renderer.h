#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "core_render/irenderer.h"

#include "shader/commontypes.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_raii.hpp>

struct MemoryBarrierInfo
{
    vk::Flags<vk::AccessFlagBits2> sourceAccess;
    vk::Flags<vk::AccessFlagBits2> destinationAccess;
    vk::Flags<vk::PipelineStageFlagBits2> sourceStage;
    vk::Flags<vk::PipelineStageFlagBits2> destinationStage;
};

class VulkanRenderer : public IRenderer
{
    public:
    bool init(IPlatform *platform) override;
    bool renderImGui() override;
    bool beginFrame() override;
    bool beginRender() override;
    bool endRender() override;
    bool drawFrame() override;
    bool present() override;
    void waitAllFramesComplete();
    bool shutdown() override;
    bool isWireframeMode() const override;
    void setWireframeMode(bool enabled) override;
    float getFrameWaitTime() override;

    vk::raii::Device &getDevice();
    vk::raii::CommandPool &getCommandPool();
    vk::raii::Queue &getQueue();
    vk::raii::PhysicalDevice &getPhysicalDevice();
    vk::raii::CommandBuffer &getCommandBuffer();

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
    void createPipelineMemoryBarrier(MemoryBarrierInfo barrierInfo);

    // TODO - incorporate this
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
        uint32_t width, uint32_t height, vk::Format format,
        vk::ImageTiling tiling, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );
    vk::raii::ImageView createImageView(
        vk::Image const &image, vk::Format format,
        vk::ImageAspectFlags aspectFlags
    );
    vk::raii::Sampler
    createSampler(vk::Filter filter, vk::SamplerAddressMode addressMode);
    
    // TODO - getter/setter
    bool enableValidationLayers_ = false;

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
    bool createDepthResources();
    bool createSyncObjects();

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
    void transitionImageLayout(
        vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        vk::ImageAspectFlags imageAspectFlags
    );
    vk::Format findSupportedFormat(
        const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
        vk::FormatFeatureFlags features
    );
    vk::Format findDepthFormat();

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

    vk::raii::Image depthImage_ = nullptr;
    vk::raii::DeviceMemory depthImageMemory_ = nullptr;
    vk::raii::ImageView depthImageView_ = nullptr;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores_;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores_;
    std::vector<vk::raii::Fence> inFlightFences_;
    uint32_t currentFrameIndex_ = MAX_FRAMES_IN_FLIGHT - 1;

    ImGuiContext *imguiContext_ = nullptr;
    vk::raii::DescriptorPool imguiDescriptorPool_ = nullptr;

    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t currentImageIndex_ = 0;

    bool wireframeMode_ = false;

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

    float frameWaitTime_;
};
