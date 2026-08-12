
#include "core_render/vulkan/image.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"

// TODO - cleanup?
void VulkanImage::init(ImageDescription imageDescription)
{
    description = imageDescription;
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    frames_.clear();
    frames_.resize(renderer_->MAX_FRAMES_IN_FLIGHT);

    if (description.createSampler)
        sampler_ = renderer_->createSampler(
            description.filter, description.addressMode
        );

    if (description.data)
    {

        vk::CommandBufferAllocateInfo allocInfo =
            vk::CommandBufferAllocateInfo{}
                .setCommandPool(renderer_->getCommandPool())
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(1);
        vk::raii::CommandBuffer commandCopyBuffer = std::move(
            renderer_->getDevice().allocateCommandBuffers(allocInfo).front()
        );
        commandCopyBuffer.begin(
            {vk::CommandBufferUsageFlagBits::eOneTimeSubmit}
        );

        for (FrameImage &frame : frames_)
        {
            frame.staging.init(renderer_, description.dataSize);
            frame.staging.upload(description.data, description.dataSize);
            std::tie(frame.image, frame.memory) = renderer_->createImage(
                description.extent.width, description.extent.height,
                description.format, vk::ImageTiling::eOptimal,
                description.usage, description.deviceMemory
            );
            frame.view = renderer_->createImageView(
                frame.image, description.format, description.aspectMask
            );

            vk::ImageMemoryBarrier barrier = {
                {},
                vk::AccessFlagBits::eTransferWrite,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eTransferDstOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                frame.image,
                vk::ImageSubresourceRange{description.aspectMask, 0, 1, 0, 1}
            };
            commandCopyBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr,
                barrier
            );

            vk::BufferImageCopy copy{
                0,
                0,
                0,
                vk::ImageSubresourceLayers{description.aspectMask, 0, 0, 1},
                {0, 0, 0},
                description.extent
            };
            commandCopyBuffer.copyBufferToImage(
                frame.staging.buffer(), frame.image,
                vk::ImageLayout::eTransferDstOptimal, copy
            );

            barrier = {
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                frame.image,
                vk::ImageSubresourceRange{description.aspectMask, 0, 1, 0, 1}
            };
            commandCopyBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr,
                nullptr, barrier
            );
        }

        commandCopyBuffer.end();
        renderer_->getQueue().submit(
            vk::SubmitInfo{}.setCommandBufferCount(1).setPCommandBuffers(
                &*commandCopyBuffer
            ),
            nullptr
        );
        renderer_->getQueue().waitIdle();
    }
}

void VulkanImage::update(vk::DeviceSize size, void *data)
{
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    FrameImage &frame = frames_[frameIndex];

    if (!frame.staging.isInitialized())
        frame.staging.init(renderer_, size);
    frame.staging.upload(data, size);

    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::ImageMemoryBarrier barrier;

    barrier = {
        vk::AccessFlagBits::eShaderRead,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        frame.image,
        vk::ImageSubresourceRange{description.aspectMask, 0, 1, 0, 1}
    };
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier
    );

    // TODO - grim
    vk::BufferImageCopy copy{
        0,         0,
        0,         vk::ImageSubresourceLayers{description.aspectMask, 0, 0, 1},
        {0, 0, 0}, description.extent
    };
    commandBuffer.copyBufferToImage(
        frame.staging.buffer(), frame.image,
        vk::ImageLayout::eTransferDstOptimal, copy
    );

    barrier = {
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        frame.image,
        vk::ImageSubresourceRange{description.aspectMask, 0, 1, 0, 1}
    };
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr,
        barrier
    );
}

vk::raii::Image *VulkanImage::getImage(uint32_t frameIndex)
{
    return &frames_[frameIndex].image;
}

vk::raii::ImageView *VulkanImage::getImageView(uint32_t frameIndex)
{
    return &frames_[frameIndex].view;
}

vk::raii::Sampler *VulkanImage::getSampler()
{
    return &sampler_;
}

FrameImage *VulkanImage::getFrameImage(uint32_t frameIndex)
{
    return &frames_[frameIndex];
}
