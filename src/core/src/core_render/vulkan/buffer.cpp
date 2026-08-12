// AI help here


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

// TODO - I don't use the frame buffer getter
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

        for (FrameBuffer &frame : frames_)
        {
            frame.staging.init(renderer_, description.bufferSize);
            std::tie(frame.buffer, frame.memory) = renderer_->createBuffer(
                bufferDescription.bufferSize, description.deviceUsage,
                description.deviceMemory
            );
            frame.mappedPointer = nullptr;

            if (bufferDescription.data)
            {
                frame.staging.upload(description.data, description.bufferSize);
                renderer_->copyBuffer(
                    frame.staging.buffer(), frame.buffer, description.bufferSize
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
    FrameBuffer &frame = frames_[frameIndex];

    if (description.persistentlyMapped)
    {
        memcpy(frame.mappedPointer, data, bufferSize);
    }
    else if (description.useStaging)
    {
        frame.staging.upload(data, bufferSize);

        vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

        vk::BufferMemoryBarrier2 stagingBarrier =
            vk::BufferMemoryBarrier2{}
                .setSrcAccessMask(vk::AccessFlagBits2::eHostWrite)
                .setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eHost)
                .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setBuffer(*frame.staging.buffer())
                .setSize(VK_WHOLE_SIZE);

        vk::DependencyInfo stagingDependency =
            vk::DependencyInfo{}
                .setBufferMemoryBarrierCount(1)
                .setPBufferMemoryBarriers(&stagingBarrier);
        commandBuffer.pipelineBarrier2(stagingDependency);

        vk::BufferCopy copy{0, 0, bufferSize};
        commandBuffer.copyBuffer(
            *frame.staging.buffer(), *frames_[frameIndex].buffer, copy
        );

        vk::BufferMemoryBarrier2 deviceBarrier =
            vk::BufferMemoryBarrier2{}
                .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                .setDstAccessMask(
                    vk::AccessFlagBits2::eShaderRead |
                    vk::AccessFlagBits2::eShaderWrite
                )
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setDstStageMask(
                    vk::PipelineStageFlagBits2::eAllGraphics |
                    vk::PipelineStageFlagBits2::eComputeShader
                )
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setBuffer(*frames_[frameIndex].buffer)
                .setSize(VK_WHOLE_SIZE);

        vk::DependencyInfo deviceDependency =
            vk::DependencyInfo{}
                .setBufferMemoryBarrierCount(1)
                .setPBufferMemoryBarriers(&deviceBarrier);
        commandBuffer.pipelineBarrier2(deviceDependency);
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
    FrameBuffer &frame = frames_[frameIndex];
    frame.staging.download(outputData, bufferSize);
}

void VulkanBuffer::copyToStaging(
    vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages,
    uint32_t frameIndex
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    FrameBuffer &frame = frames_[frameIndex];

    // Shader write -> transfer read (device buffer)
    vk::BufferMemoryBarrier2 deviceBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
            .setSrcStageMask(
                vk::PipelineStageFlagBits2::eComputeShader |
                vk::PipelineStageFlagBits2::eAllGraphics
            )
            .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setBuffer(*frames_[frameIndex].buffer)
            .setSize(VK_WHOLE_SIZE);

    vk::DependencyInfo deviceDependency =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&deviceBarrier);
    commandBuffer.pipelineBarrier2(deviceDependency);

    vk::BufferCopy copy{0, 0, size};
    commandBuffer.copyBuffer(
        *frames_[frameIndex].buffer, *frame.staging.buffer(), copy
    );

    // Transfer write -> host read (staging buffer)
    vk::BufferMemoryBarrier2 stagingBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eHostRead)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstStageMask(vk::PipelineStageFlagBits2::eHost)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setBuffer(*frame.staging.buffer())
            .setSize(VK_WHOLE_SIZE);

    vk::DependencyInfo stagingDependency =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&stagingBarrier);
    commandBuffer.pipelineBarrier2(stagingDependency);
}

void *VulkanBuffer::getMappedPointer(uint32_t frameIndex)
{
    return frames_[frameIndex].mappedPointer;
}

FrameBuffer *VulkanBuffer::getFrameBuffer(uint32_t frameIndex)
{
    return &frames_[frameIndex];
}
