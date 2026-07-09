#include "core_render/vulkan/buffer.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"

BufferDescription UniformBuffer{
    .descriptorType = vk::DescriptorType::eUniformBuffer,
    .deviceUsage = vk::BufferUsageFlagBits::eUniformBuffer,
    .deviceMemory = vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent,
    .stageFlags = {},
    .persistentlyMapped = true
};

BufferDescription StorageRead{
    .descriptorType = vk::DescriptorType::eStorageBuffer,
    .deviceUsage = vk::BufferUsageFlagBits::eStorageBuffer |
                   vk::BufferUsageFlagBits::eTransferDst,
    .stagingUsage = vk::BufferUsageFlagBits::eTransferSrc,
    .deviceMemory = vk::MemoryPropertyFlagBits::eDeviceLocal,
    .stageFlags = {},
    .useStaging = true
};

BufferDescription StorageReadWrite{
    .descriptorType = vk::DescriptorType::eStorageBuffer,
    .deviceUsage = vk::BufferUsageFlagBits::eStorageBuffer |
                   vk::BufferUsageFlagBits::eTransferDst |
                   vk::BufferUsageFlagBits::eTransferSrc,
    .stagingUsage = vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eTransferSrc,
    .deviceMemory = vk::MemoryPropertyFlagBits::eDeviceLocal,
    .stageFlags = {},
    .useStaging = true,
    .readable = true
};

BufferDescription VertexBuffer{
    .deviceUsage = vk::BufferUsageFlagBits::eVertexBuffer |
                   vk::BufferUsageFlagBits::eTransferDst,
    .stagingUsage = vk::BufferUsageFlagBits::eTransferSrc,
    .deviceMemory = vk::MemoryPropertyFlagBits::eDeviceLocal,
    .stageFlags = {vk::ShaderStageFlagBits::eVertex},
    .useStaging = true
};

BufferDescription IndexBuffer{
    .deviceUsage = vk::BufferUsageFlagBits::eIndexBuffer |
                   vk::BufferUsageFlagBits::eTransferDst,
    .stagingUsage = vk::BufferUsageFlagBits::eTransferSrc,
    .deviceMemory = vk::MemoryPropertyFlagBits::eDeviceLocal,
    .stageFlags = {vk::ShaderStageFlagBits::eVertex},
    .useStaging = true
};

void VulkanBuffer::init(BufferDescription bufferDescription)
{
    description = bufferDescription;
    bufferSize_ = bufferDescription.bufferSize;
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    stagingBuffer_ = nullptr;
    stagingBufferMemory_ = nullptr;
    frames_.clear();

    if (description.useStaging)
    {
        // Create shared staging buffer (host-visible)
        std::tie(stagingBuffer_, stagingBufferMemory_) =
            renderer_->createBuffer(
                bufferDescription.bufferSize, description.stagingUsage,
                vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent
            );

        // Create per-frame device buffers (device-local)
        frames_.resize(renderer_->MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < renderer_->MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::tie(frames_[i].buffer, frames_[i].memory) =
                renderer_->createBuffer(
                    bufferDescription.bufferSize, description.deviceUsage,
                    description.deviceMemory
                );
            frames_[i].mappedPointer = nullptr;

            if (bufferDescription.data)
            {
                void *stagingData = stagingBufferMemory_.mapMemory(
                    0, bufferDescription.bufferSize
                );
                memcpy(
                    stagingData, bufferDescription.data,
                    bufferDescription.bufferSize
                );
                stagingBufferMemory_.unmapMemory();
                renderer_->copyBuffer(
                    stagingBuffer_, frames_[i].buffer,
                    bufferDescription.bufferSize
                );
            }
        }
    }
    else if (description.persistentlyMapped)
    {
        // Create per-frame persistently mapped buffers
        frames_.resize(renderer_->MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < renderer_->MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::tie(frames_[i].buffer, frames_[i].memory) =
                renderer_->createBuffer(
                    bufferDescription.bufferSize, description.deviceUsage,
                    description.deviceMemory
                );
            frames_[i].mappedPointer =
                frames_[i].memory.mapMemory(0, bufferDescription.bufferSize);
            if (bufferDescription.data)
            {
                memcpy(
                    frames_[i].mappedPointer, bufferDescription.data,
                    bufferDescription.bufferSize
                );
            }
        }
    }
    else
    {
        // Simple buffer with no staging, no mapping — still per-frame
        frames_.resize(renderer_->MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < renderer_->MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::tie(frames_[i].buffer, frames_[i].memory) =
                renderer_->createBuffer(
                    bufferDescription.bufferSize, description.deviceUsage,
                    description.deviceMemory
                );
            frames_[i].mappedPointer = nullptr;
        }
    }
}

void VulkanBuffer::update(vk::DeviceSize bufferSize, void *data)
{
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();

    if (description.persistentlyMapped)
    {
        // Direct write to current frame's mapped memory
        memcpy(frames_[frameIndex].mappedPointer, data, bufferSize);
    }
    else if (description.useStaging)
    {
        // Copy through staging buffer with proper barriers
        vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

        // Map staging buffer and copy data
        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();

        // Barrier: host write -> transfer read
        vk::BufferMemoryBarrier barrier{
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eTransferRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            stagingBuffer_,
            0,
            VK_WHOLE_SIZE
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eHost,
            vk::PipelineStageFlagBits::eTransfer, {}, nullptr, barrier, nullptr
        );

        // Copy staging -> current frame's device buffer
        vk::BufferCopy copy{0, 0, bufferSize};
        commandBuffer.copyBuffer(
            stagingBuffer_, frames_[frameIndex].buffer, copy
        );

        // Barrier: transfer write -> shader read
        vk::BufferMemoryBarrier barrier2{
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eShaderRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            frames_[frameIndex].buffer,
            0,
            VK_WHOLE_SIZE
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eAllGraphics, {}, nullptr, barrier2,
            nullptr
        );
    }
}

vk::raii::Buffer *VulkanBuffer::getBuffer(uint32_t frameIndex)
{
    return &frames_[frameIndex].buffer;
}

vk::raii::DeviceMemory *VulkanBuffer::getDeviceMemory(uint32_t frameIndex)
{
    return &frames_[frameIndex].memory;
}

void VulkanBuffer::readData(vk::DeviceSize bufferSize, void *outputData)
{
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    copyToStaging(
        bufferSize, vk::PipelineStageFlagBits::eAllGraphics, frameIndex
    );
    void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
    memcpy(outputData, stagingData, bufferSize);
    stagingBufferMemory_.unmapMemory();
}

void VulkanBuffer::copyToStaging(
    vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages,
    uint32_t frameIndex
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

    vk::BufferMemoryBarrier barrier{
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eTransferRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        frames_[frameIndex].buffer,
        0,
        VK_WHOLE_SIZE
    };

    commandBuffer.pipelineBarrier(
        pipelineStages, vk::PipelineStageFlagBits::eTransfer, {}, nullptr,
        barrier, nullptr
    );

    vk::BufferCopy copy{0, 0, size};
    commandBuffer.copyBuffer(frames_[frameIndex].buffer, stagingBuffer_, copy);

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

void *VulkanBuffer::getMappedPointer(uint32_t frameIndex)
{
    return frames_[frameIndex].mappedPointer;
}
