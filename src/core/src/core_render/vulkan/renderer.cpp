#include "core_render/vulkan/renderer.h"
#include "core_platform/linux/platform.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <spdlog/spdlog.h>

bool VulkanRenderer::init(IPlatform *platform)
{
    spdlog::debug("Initialising Vulkan renderer");
    platform_ = platform;
    width_ = platform->getConfig().window_width;
    height_ = platform->getConfig().window_height;

    try
    {
        createInstance();
        setupDebugMessenger();
        createSurface();
        selectGPU();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        if (!initImGui())
            throw std::runtime_error("ImGui failed to initialise");
        createSynchronizationPrimitives();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createDescriptorPool();

        spdlog::info("VulkanRenderer initialised successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        spdlog::error("VulkanRenderer init failed: {}", e.what());
        return false;
    }
}

void VulkanRenderer::createInstance()
{
    spdlog::debug("Creating Vulkan instance");
}

void VulkanRenderer::setupDebugMessenger()
{
    spdlog::debug("Setting up debug messenger");
}

void VulkanRenderer::createSurface()
{
    spdlog::debug("Creating surface");
}

void VulkanRenderer::selectGPU()
{
    spdlog::debug("Selecting GPU");
}

void VulkanRenderer::createLogicalDevice()
{
    spdlog::debug("Creating logical device");
}

void VulkanRenderer::createSwapchain()
{
    spdlog::debug("Creating swap chain");
}

void VulkanRenderer::createImageViews()
{
    spdlog::debug("Creating image views");
}

void VulkanRenderer::createRenderPass()
{
    spdlog::debug("Creating render pass");
}

void VulkanRenderer::createFramebuffers()
{
    spdlog::debug("Creating framebuffers");
}

void VulkanRenderer::createCommandPool()
{
    spdlog::debug("Creating command pool");
}

void VulkanRenderer::createCommandBuffers()
{
    spdlog::debug("Creating command buffers");
}

void VulkanRenderer::createSynchronizationPrimitives()
{
    spdlog::debug("Creating synchronization primitives");
}

bool VulkanRenderer::beginFrame()
{
    return true;
}

bool VulkanRenderer::endFrame()
{
    return true;
}

void VulkanRenderer::createDescriptorPool()
{
}

bool VulkanRenderer::initImGui()
{
    imguiContext_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext_);

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    GLFWwindow *glfwWindow =
        static_cast<PlatformLinux *>(platform_)->getGLFWWindow();
    ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

    VkDescriptorPoolSize poolSizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000 * sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.poolSizeCount =
        static_cast<uint32_t>(sizeof(poolSizes) / sizeof(poolSizes[0]));
    poolInfo.pPoolSizes = poolSizes;

    VkResult result = vkCreateDescriptorPool(
        device_, &poolInfo, nullptr, &imguiDescriptorPool_
    );
    if (result != VK_SUCCESS)
    {
        spdlog::error(
            "Failed to create ImGui descriptor pool: {}",
            static_cast<int>(result)
        );
        imguiDescriptorPool_ = VK_NULL_HANDLE;
        return false;
    }

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance_;
    initInfo.PhysicalDevice = physicalDevice_;
    initInfo.Device = device_;
    initInfo.QueueFamily = findQueueFamilyIndex();
    initInfo.Queue = graphicsQueue_;
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = imguiDescriptorPool_;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(swapchainImages_.size());
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.RenderPass = renderPass_;

    ImGui_ImplVulkan_Init(&initInfo);

    // Upload font texture
    ImGui_ImplVulkan_CreateFontsTexture();

    spdlog::info("ImGui initialised");
    return true;
}

bool VulkanRenderer::frame()
{
    return true;
}

bool VulkanRenderer::renderImGui()
{
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();
    if (drawData->TotalVtxCount > 0)
    {
        ImGui_ImplVulkan_RenderDrawData(
            drawData, commandBuffers_[currentImageIndex_]
        );
    }

    return true;
}

void VulkanRenderer::shutdownImGui()
{
    if (imguiContext_)
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imguiContext_);
        imguiContext_ = nullptr;

        if (imguiDescriptorPool_ != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device_, imguiDescriptorPool_, nullptr);
            imguiDescriptorPool_ = VK_NULL_HANDLE;
        }

        spdlog::info("ImGui shut down");
    }
}

uint32_t VulkanRenderer::findQueueFamilyIndex() const
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice_, &queueFamilyCount, nullptr
    );
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice_, &queueFamilyCount, queueFamilies.data()
    );

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            return i;
    }
    throw std::runtime_error("Failed to find graphics queue family");
}
