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
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    frames_.clear();
    frames_.resize(renderer_->MAX_FRAMES_IN_FLIGHT);

    if (description.useStaging)
    {
        staging_.init(renderer_, description.bufferSize);

        for (FrameBuffer &frame : frames_)
        {
            std::tie(frame.buffer, frame.memory) = renderer_->createBuffer(
                bufferDescription.bufferSize, description.deviceUsage,
                description.deviceMemory
            );
            frame.mappedPointer = nullptr;

            if (bufferDescription.data)
            {
                staging_.upload(description.data, description.bufferSize);
                renderer_->copyBuffer(
                    staging_.buffer(), frame.buffer, description.bufferSize
                );
            }
        }
    }
    else if (description.persistentlyMapped)
    {
        for (FrameBuffer &frame : frames_)
        {
            std::tie(frame.buffer, frame.memory) = renderer_->createBuffer(
                bufferDescription.bufferSize, description.deviceUsage,
                description.deviceMemory
            );
            frame.mappedPointer =
                frame.memory.mapMemory(0, bufferDescription.bufferSize);
            if (bufferDescription.data)
            {
                memcpy(
                    frame.mappedPointer, bufferDescription.data,
                    bufferDescription.bufferSize
                );
            }
        }
    }
    else
    {
        for (FrameBuffer &frame : frames_)
        {
            std::tie(frame.buffer, frame.memory) = renderer_->createBuffer(
                bufferDescription.bufferSize, description.deviceUsage,
                description.deviceMemory
            );
            frame.mappedPointer = nullptr;
        }
    }
}

// TODO - refactor this (ie use functions for barrier add and set
// TODO - this does not feel general at all (eg read/write and all pipeline
// stages)
void VulkanBuffer::update(vk::DeviceSize bufferSize, void *data)
{
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();

    if (description.persistentlyMapped)
    {
        memcpy(frames_[frameIndex].mappedPointer, data, bufferSize);
    }
    else if (description.useStaging)
    {
        staging_.upload(data, bufferSize);

        vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

        vk::BufferMemoryBarrier barrier{
            vk::AccessFlagBits::eHostWrite,
            vk::AccessFlagBits::eTransferRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            staging_.buffer(),
            0,
            VK_WHOLE_SIZE
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eHost,
            vk::PipelineStageFlagBits::eTransfer, {}, nullptr, barrier, nullptr
        );

        vk::BufferCopy copy{0, 0, bufferSize};
        commandBuffer.copyBuffer(
            staging_.buffer(), frames_[frameIndex].buffer, copy
        );

        vk::BufferMemoryBarrier barrier2{
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            frames_[frameIndex].buffer,
            0,
            VK_WHOLE_SIZE
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eAllGraphics |
                vk::PipelineStageFlagBits::eComputeShader,
            {}, nullptr, barrier2, nullptr
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
    staging_.download(outputData, bufferSize);
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
    commandBuffer.copyBuffer(
        frames_[frameIndex].buffer, staging_.buffer(), copy
    );

    vk::BufferMemoryBarrier barrier2{
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eHostRead,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        staging_.buffer(),
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

FrameBuffer *VulkanBuffer::getFrameBuffer(uint32_t frameIndex)
{
    return &frames_[frameIndex];
}
