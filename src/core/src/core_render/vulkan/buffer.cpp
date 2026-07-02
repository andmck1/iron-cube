// TODO - intense levels of abstraction here?
#include "core_render/vulkan/buffer.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"

void VulkanBufferBase::update(vk::DeviceSize bufferSize, void *data)
{
    copyBuffer(stagingBuffer_, buffer_, stagingBufferMemory_, bufferSize, data);
}

vk::raii::Buffer *VulkanBufferBase::getBuffer()
{
    return &buffer_;
}

vk::raii::DeviceMemory *VulkanBufferBase::getDeviceMemory()
{
    return &bufferMemory_;
}

void VulkanBufferBase::copyBuffer(
    vk::raii::Buffer &fromBuffer, vk::raii::Buffer &toBuffer,
    vk::raii::DeviceMemory &fromBufferMemory, vk::DeviceSize bufferSize,
    void *data
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    void *bufferData = fromBufferMemory.mapMemory(0, bufferSize);
    memcpy(bufferData, data, bufferSize);
    fromBufferMemory.unmapMemory();

    vk::BufferMemoryBarrier barrier{
        vk::AccessFlagBits::eHostWrite,
        vk::AccessFlagBits::eTransferRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        fromBuffer,
        0,
        VK_WHOLE_SIZE
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer,
        {}, nullptr, barrier, nullptr
    );

    vk::BufferCopy copy{0, 0, bufferSize};

    // TODO - I don't need this for all copies, only ones that use this buffer
    // later
    commandBuffer.copyBuffer(fromBuffer, toBuffer, copy);

    vk::BufferMemoryBarrier barrier2{
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead |
            vk::AccessFlagBits::eVertexAttributeRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        toBuffer,
        0,
        VK_WHOLE_SIZE
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eVertexInput |
            vk::PipelineStageFlagBits::eVertexShader,
        {}, nullptr, barrier2, nullptr
    );
}

// TODO - I'm using staging buffer here for vertex and index, which I don't want
// to do as they're static and this wastes memory
void VulkanBufferSingleStageBase::init(vk::DeviceSize bufferSize, void *data)
{

    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::tie(stagingBuffer_, stagingBufferMemory_) = renderer_->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    std::tie(buffer_, bufferMemory_) = renderer_->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferDst | bufferUsageFlags_,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    if (data)
    {
        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();
        renderer_->copyBuffer(stagingBuffer_, buffer_, bufferSize);
    }
}

void VulkanBufferUniform::init(
    vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags_
)
{
    stageFlags = stageFlags_;

    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::tie(buffer_, bufferMemory_) = renderer_->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );
    mappedPointer_ = bufferMemory_.mapMemory(0, bufferSize);
    if (data)
    {
        memcpy(mappedPointer_, data, bufferSize);
    }
}

void VulkanBufferUniform::update(vk::DeviceSize bufferSize, void *data)
{
    memcpy(mappedPointer_, data, bufferSize);
}

void VulkanBufferStorageBase::init(
    vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags_
)
{
    stageFlags = stageFlags_;

    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::tie(stagingBuffer_, stagingBufferMemory_) = renderer_->createBuffer(
        bufferSize, stagingBufferUsageFlags_,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    std::tie(buffer_, bufferMemory_) = renderer_->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eStorageBuffer | bufferUsageFlags_,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    if (data)
    {
        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();
        renderer_->copyBuffer(stagingBuffer_, buffer_, bufferSize);
    }
}

// TODO - copied code here
void VulkanBufferRWStorage::copyToStaging(
    vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

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
        pipelineStages, vk::PipelineStageFlagBits::eTransfer, {}, nullptr,
        barrier, nullptr
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

void VulkanBufferRWStorage::readData(
    vk::DeviceSize bufferSize, void *outputData
)
{
    void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
    memcpy(outputData, stagingData, bufferSize);
    stagingBufferMemory_.unmapMemory();
}

void *VulkanBufferUniform::getMappedPointer()
{
    return mappedPointer_;
}
