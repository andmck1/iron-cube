// AI help here

#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>
#include <variant>

std::optional<vk::raii::Pipeline> &VulkanShaderPipeline::getGraphicsPipeline()
{
    return graphicsPipeline_;
}

void VulkanShaderPipeline::addResource(const ResourceDescription &description)
{
    uint slotNumber = std::visit(
        [](const auto &desc) { return desc.bufferSlot; }, description
    );
    if ((slotNumber + 1) > resources_.size())
        resources_.resize(slotNumber + 1);
    resources_[slotNumber].init(description);
}

void VulkanShaderPipeline::addResourceWrite(
    PipelineResource &resource, vk::DeviceSize bufferSize,
    const ResourceDescription &description, uint32_t frameIndex,
    vk::raii::DescriptorSet &descriptorSet,
    std::vector<vk::WriteDescriptorSet> &descriptorWrites,
    std::vector<vk::DescriptorBufferInfo> &bufferInfos,
    std::vector<vk::DescriptorImageInfo> &imageInfos
)
{
    uint resourceSlot = std::visit(
        [](const auto &desc) { return desc.bufferSlot; }, description
    );
    vk::DescriptorType descriptorType = std::visit(
        [](const auto &desc) { return desc.descriptorType; }, description
    );

    if (const auto *buffer = std::get_if<BufferDescription>(&description))
    {
        bufferInfos.emplace_back(
            vk::DescriptorBufferInfo{}
                .setBuffer(
                    *resource.asBuffer()->getFrameBuffer(frameIndex)->buffer
                )
                .setOffset(0)
                .setRange(bufferSize)
        );

        descriptorWrites.emplace_back(
            vk::WriteDescriptorSet{}
                .setDstSet(descriptorSet)
                .setDstBinding(static_cast<uint32_t>(resourceSlot))
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(descriptorType)
                .setPBufferInfo(&bufferInfos.back())
        );
    }
    else if (const auto *image = std::get_if<ImageDescription>(&description))
    {
        VulkanImage *vulkanImage = resource.asImage();
        FrameImage *frameImage = vulkanImage->getFrameImage(frameIndex);

        vk::Sampler samplerHandle = nullptr;
        if (vulkanImage->getSampler())
        {
            samplerHandle = *vulkanImage->getSampler();
        }

        imageInfos.emplace_back(
            vk::DescriptorImageInfo{}
                .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setImageView(*frameImage->view)
                .setSampler(samplerHandle)
        );

        descriptorWrites.emplace_back(
            vk::WriteDescriptorSet{}
                .setDstSet(descriptorSet)
                .setDstBinding(static_cast<uint32_t>(resourceSlot))
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(descriptorType)
                .setPImageInfo(&imageInfos.back())
        );
    }
}

bool VulkanShaderPipeline::initDescriptorLayout()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const ResourceDescription &description : resourceDescriptions_)
    {
        uint bufferSlot = std::visit(
            [](const auto &desc) { return desc.bufferSlot; }, description
        );
        vk::DescriptorType descriptorType = std::visit(
            [](const auto &desc) { return desc.descriptorType; }, description
        );
        vk::Flags stageFlags = std::visit(
            [](const auto &desc) { return desc.stageFlags; }, description
        );

        bindings.emplace_back(
            vk::DescriptorSetLayoutBinding{}
                .setBinding(static_cast<uint32_t>(bufferSlot))
                .setDescriptorType(descriptorType)
                .setDescriptorCount(1)
                .setStageFlags(stageFlags)
        );
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo =
        vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount(static_cast<uint32_t>(bindings.size()))
            .setPBindings(bindings.data());

    descriptorSetLayout_ =
        vk::raii::DescriptorSetLayout(renderer->getDevice(), layoutInfo);
    return true;
}

bool VulkanShaderPipeline::createDescriptorPool()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    // Count descriptors by type
    uint32_t uniformCount = 0;
    uint32_t storageCount = 0;
    uint32_t imageCount = 0;
    for (const ResourceDescription &description : resourceDescriptions_)
    {
        vk::DescriptorType descriptorType = std::visit(
            [](const auto &desc) { return desc.descriptorType; }, description
        );
        if (descriptorType == vk::DescriptorType::eUniformBuffer)
        {
            uniformCount++;
        }
        else if (descriptorType == vk::DescriptorType::eStorageBuffer)
        {
            storageCount++;
        }
        else if (descriptorType == vk::DescriptorType::eCombinedImageSampler)
        {
            imageCount++;
        }
    }

    std::vector<vk::DescriptorPoolSize> poolSizes;
    if (uniformCount > 0)
    {
        poolSizes.emplace_back(
            vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(
                    renderer->MAX_FRAMES_IN_FLIGHT * uniformCount
                )
        );
    }
    if (storageCount > 0)
    {
        poolSizes.emplace_back(
            vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eStorageBuffer)
                .setDescriptorCount(
                    renderer->MAX_FRAMES_IN_FLIGHT * storageCount
                )
        );
    }
    if (imageCount > 0)
    {
        poolSizes.emplace_back(
            vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eCombinedImageSampler)
                .setDescriptorCount(renderer->MAX_FRAMES_IN_FLIGHT * imageCount)
        );
    }

    vk::DescriptorPoolCreateInfo poolInfo =
        vk::DescriptorPoolCreateInfo{}
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(renderer->MAX_FRAMES_IN_FLIGHT)
            .setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
            .setPPoolSizes(poolSizes.data());

    descriptorPool_ = vk::raii::DescriptorPool(renderer->getDevice(), poolInfo);
    return true;
}

bool VulkanShaderPipeline::initDescriptorSets()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    if (!createDescriptorPool())
        return false;

    std::vector<vk::DescriptorSetLayout> layouts(
        renderer->MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout_
    );
    vk::DescriptorSetAllocateInfo allocInfo =
        vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(descriptorPool_)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

    descriptorSets_ = renderer->getDevice().allocateDescriptorSets(allocInfo);

    for (uint32_t frameIndex = 0; frameIndex < renderer->MAX_FRAMES_IN_FLIGHT;
         frameIndex++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        std::vector<vk::DescriptorImageInfo> imageInfos;

        size_t totalResources = resourceDescriptions_.size();
        descriptorWrites.reserve(totalResources);
        bufferInfos.reserve(totalResources);
        imageInfos.reserve(totalResources);

        for (const ResourceDescription &description : resourceDescriptions_)
        {
            PipelineResource &resource = resources_[std::visit(
                [](const auto &desc) { return desc.bufferSlot; }, description
            )];

            addResourceWrite(
                resource, VK_WHOLE_SIZE, description, frameIndex,
                descriptorSets_[frameIndex], descriptorWrites, bufferInfos,
                imageInfos
            );
        }

        renderer->getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    return true;
}

void VulkanShaderPipeline::shutdownBuffers()
{
    resourceDescriptions_.clear();
    resources_.clear();
}

void VulkanShaderPipeline::addBarrier(BufferBarrierInfo barrierInfo)
{
    barriers_.emplace_back(
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(barrierInfo.sourceAccess)
            .setDstAccessMask(barrierInfo.destinationAccess)
            .setSrcStageMask(barrierInfo.sourceStage)
            .setDstStageMask(barrierInfo.destinationStage)
            .setBuffer(*barrierInfo.buffer)
            .setSize(VK_WHOLE_SIZE)
    );
}

void VulkanShaderPipeline::setBarrier()
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::DependencyInfo barrierDependencyInfo =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(barriers_.size())
            .setPBufferMemoryBarriers(barriers_.data());
    commandBuffer.pipelineBarrier2(barrierDependencyInfo);
    barriers_.clear();
}
