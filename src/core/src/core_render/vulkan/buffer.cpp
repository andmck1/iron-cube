#include "core_render/vulkan/buffer.h"

bool VulkanUniformBuffer::init(
    vk::DeviceSize bufferSize, vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties
)
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    auto [buffer, bufferMem] =
        renderer->createBuffer(bufferSize, usage, properties);
    buffer_ = std::move(buffer);
    bufferMemory_ = std::move(bufferMem);
    mappedPointer_ = bufferMemory_.mapMemory(0, bufferSize);

    return true;
}

vk::raii::Buffer *VulkanUniformBuffer::getBuffer()
{
    return &buffer_;
}

void *VulkanUniformBuffer::getMappedPointer()
{
    return mappedPointer_;
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
