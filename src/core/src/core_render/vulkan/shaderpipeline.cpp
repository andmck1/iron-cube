#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

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

void VulkanBuffer::copyToStaging(vk::DeviceSize size)
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::raii::CommandBuffer &commandBuffer = renderer->getCommandBuffer();

    vk::BufferMemoryBarrier barrier{
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eTransferRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        buffer_,
        0,
        VK_WHOLE_SIZE
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTaskShaderEXT,
        vk::PipelineStageFlagBits::eTransfer, {}, nullptr, barrier, nullptr
    );

    vk::BufferCopy copy{0, 0, size};
    commandBuffer.copyBuffer(buffer_, stagingBuffer_, copy);

    vk::BufferMemoryBarrier barrier2{
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eHostRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        stagingBuffer_,
        0,
        VK_WHOLE_SIZE
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eHost,
        {}, nullptr, barrier2, nullptr
    );
}
