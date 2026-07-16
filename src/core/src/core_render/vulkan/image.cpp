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
        staging_.init(renderer_, description.dataSize);
        staging_.upload(description.data, description.dataSize);

        vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
        vk::ImageMemoryBarrier barrier;

        for (FrameImage &frame : frames_)
        {
            std::tie(frame.image, frame.memory) = renderer_->createImage(
                description.extent.width, description.extent.height,
                description.format, vk::ImageTiling::eOptimal,
                description.usage, description.deviceMemory
            );
            frame.view = renderer_->createImageView(
                frame.image, description.format, description.aspectMask
            );

            barrier = {
                {},
                vk::AccessFlagBits::eTransferWrite,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eTransferDstOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                frame.image,
                vk::ImageSubresourceRange{description.aspectMask, 0, 1, 0, 1}
            };
            commandBuffer.pipelineBarrier(
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
            commandBuffer.copyBufferToImage(
                staging_.buffer(), frame.image,
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
                vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr,
                nullptr, barrier
            );
        }
    }
}

void VulkanImage::update(vk::DeviceSize size, void *data)
{
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();

    if (!staging_.isInitialized())
        staging_.init(renderer_, size);

    staging_.upload(data, size);

    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::ImageMemoryBarrier barrier;
    FrameImage &frame = frames_[frameIndex];

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
        staging_.buffer(), frame.image, vk::ImageLayout::eTransferDstOptimal,
        copy
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
