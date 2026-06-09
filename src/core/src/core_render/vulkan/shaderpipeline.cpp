#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>
#include <vector>

std::optional<vk::raii::Pipeline> &VulkanShaderPipeline::getGraphicsPipeline()
{
    return graphicsPipeline_;
}

vk::raii::Buffer *VulkanBuffer::getBuffer()
{
    return &buffer_;
}

vk::raii::DeviceMemory *VulkanBuffer::getDeviceMemory()
{
    return &bufferMemory_;
}
