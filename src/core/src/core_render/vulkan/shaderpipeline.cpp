#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

std::optional<vk::raii::Pipeline> &VulkanShaderPipeline::getGraphicsPipeline()
{
    return graphicsPipeline_;
}

void VulkanShaderPipeline::addBufferWrite(
    vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
    vk::DescriptorType bufferType, vk::raii::Buffer *buffer,
    vk::DeviceSize bufferSize,
    std::vector<vk::WriteDescriptorSet> &descriptorWrites,
    std::vector<vk::DescriptorBufferInfo> &bufferInfos
)
{
    bufferInfos.emplace_back(
        vk::DescriptorBufferInfo{}.setBuffer(*buffer).setOffset(0).setRange(
            bufferSize
        )
    );
    descriptorWrites.emplace_back(
        vk::WriteDescriptorSet{}
            .setDstSet(descriptorSet)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(bufferType)
            .setPBufferInfo(&bufferInfos.back())
    );
}

bool VulkanShaderPipeline::initDescriptorLayout()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    for (const BufferDescription &desc : bufferDescriptions_)
    {
        bindings.emplace_back(
            vk::DescriptorSetLayoutBinding{}
                .setBinding(static_cast<uint32_t>(desc.bufferSlot))
                .setDescriptorType(desc.descriptorType)
                .setDescriptorCount(1)
                .setStageFlags(desc.stageFlags)
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
    for (const BufferDescription &desc : bufferDescriptions_)
    {
        if (desc.descriptorType == vk::DescriptorType::eUniformBuffer)
        {
            uniformCount++;
        }
        else if (desc.descriptorType == vk::DescriptorType::eStorageBuffer)
        {
            storageCount++;
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

        size_t totalBuffers = bufferDescriptions_.size();
        descriptorWrites.reserve(totalBuffers);
        bufferInfos.reserve(totalBuffers);

        for (const BufferDescription &bufferDescription : bufferDescriptions_)
        {
            VulkanBuffer &buffer = buffers_[bufferDescription.bufferSlot];

            addBufferWrite(
                descriptorSets_[frameIndex],
                static_cast<uint32_t>(bufferDescription.bufferSlot),
                bufferDescription.descriptorType, buffer.getBuffer(frameIndex),
                VK_WHOLE_SIZE, descriptorWrites, bufferInfos
            );
        }

        renderer->getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    return true;
}

void VulkanShaderPipeline::shutdownBuffers()
{
    bufferDescriptions_.clear();
    buffers_.clear();
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
