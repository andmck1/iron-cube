#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/renderer.h"
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
    // TODO - can I get rid of repeated dynamic cast here?
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    uint32_t bindingIndex = 0;

    initBufferDescriptorLayout<VulkanBufferUniform>(
        uniformBuffers_, bindingIndex, vk::DescriptorType::eUniformBuffer,
        &bindings
    );
    initBufferDescriptorLayout<VulkanBufferRWStorage>(
        storageRWBuffers_, bindingIndex, vk::DescriptorType::eStorageBuffer,
        &bindings
    );
    initBufferDescriptorLayout<VulkanBufferRStorage>(
        storageRBuffers_, bindingIndex, vk::DescriptorType::eStorageBuffer,
        &bindings
    );

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

    std::vector<vk::DescriptorPoolSize> poolSizes;
    if (uniformBuffers_.size() > 0)
    {
        poolSizes.emplace_back(
            vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(
                    renderer->MAX_FRAMES_IN_FLIGHT * uniformBuffers_.size()
                )
        );
    }

    uint totalNStorageBuffers =
        storageRBuffers_.size() + storageRWBuffers_.size();
    if (totalNStorageBuffers > 0)
    {
        poolSizes.emplace_back(
            vk::DescriptorPoolSize{}
                .setType(vk::DescriptorType::eStorageBuffer)
                .setDescriptorCount(
                    renderer->MAX_FRAMES_IN_FLIGHT * totalNStorageBuffers
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

    for (size_t frameIndex = 0; frameIndex < renderer->MAX_FRAMES_IN_FLIGHT;
         frameIndex++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        std::vector<vk::DescriptorBufferInfo> bufferInfos;

        uint totalNStorageBuffers =
            storageRBuffers_.size() + storageRWBuffers_.size();
        size_t totalBuffers = uniformBuffers_.size() + totalNStorageBuffers;
        descriptorWrites.reserve(totalBuffers);
        bufferInfos.reserve(totalBuffers);

        uint32_t bindingIndex = 0;

        initBufferDescriptorSet<VulkanBufferUniform>(
            uniformBuffers_, frameIndex, vk::DescriptorType::eUniformBuffer,
            bindingIndex, &descriptorWrites, &bufferInfos
        );
        initBufferDescriptorSet<VulkanBufferRWStorage>(
            storageRWBuffers_, frameIndex, vk::DescriptorType::eStorageBuffer,
            bindingIndex, &descriptorWrites, &bufferInfos
        );
        initBufferDescriptorSet<VulkanBufferRStorage>(
            storageRBuffers_, frameIndex, vk::DescriptorType::eStorageBuffer,
            bindingIndex, &descriptorWrites, &bufferInfos
        );

        renderer->getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    return true;
}

void VulkanShaderPipeline::shutdownBuffers()
{
    uniformBuffers_.clear();
    storageRWBuffers_.clear();
    storageRBuffers_.clear();
}
