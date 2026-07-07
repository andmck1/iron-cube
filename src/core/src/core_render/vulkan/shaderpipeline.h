#pragma once

#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/buffer.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    template <typename BufferType>
    void addBuffer(
        std::vector<std::vector<BufferType>> &buffers,
        vk::DeviceSize bufferSize, void *data,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    )
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );
        buffers.push_back({});
        for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
        {
            buffers.back().emplace_back();
            buffers.back().back().init(bufferSize, data, stageFlags);
        }
    }

    void addBufferWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        vk::DescriptorType bufferType, vk::raii::Buffer *buffer,
        vk::DeviceSize bufferSize,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );
    template <typename BufferType>
    void initBufferDescriptorLayout(
        std::vector<std::vector<BufferType>> &buffers, uint32_t &bindingIndex,
        vk::DescriptorType vulkanDescriptorType,
        std::vector<vk::DescriptorSetLayoutBinding> *bindings
    )
    {
        for (const std::vector<BufferType> &frameBuffers : buffers)
        {
            const BufferType &buffer = frameBuffers.front();
            bindings->emplace_back(
                vk::DescriptorSetLayoutBinding{}
                    .setBinding(bindingIndex)
                    .setDescriptorType(vulkanDescriptorType)
                    .setDescriptorCount(1)
                    .setStageFlags(buffer.stageFlags)
            );
            bindingIndex++;
        }
    }

    bool initDescriptorLayout();
    template <typename BufferType>
    void initBufferDescriptorSet(
        std::vector<std::vector<BufferType>> &buffers, size_t frameIndex,
        vk::DescriptorType vulkanDescriptorType, uint32_t &bindingIndex,
        std::vector<vk::WriteDescriptorSet> *descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> *bufferInfos
    )
    {
        for (size_t bufferIndex = 0; bufferIndex < buffers.size();
             bufferIndex++)
        {
            addBufferWrite(
                descriptorSets_[frameIndex], bindingIndex, vulkanDescriptorType,
                buffers[bufferIndex][frameIndex].getBuffer(), VK_WHOLE_SIZE,
                *descriptorWrites, *bufferInfos
            );
            bindingIndex++;
        }
    }

    bool initDescriptorSets();
    bool createDescriptorPool();
    void shutdownBuffers();

    virtual void initUniformBuffers() = 0;
    virtual void initStorageBuffers() = 0;

    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;

    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    std::vector<std::vector<VulkanBufferUniform>> uniformBuffers_;
    std::vector<std::vector<VulkanBufferRStorage>> storageRBuffers_;
    std::vector<std::vector<VulkanBufferRWStorage>> storageRWBuffers_;
    std::vector<std::vector<VulkanBufferIndirectStorage>>
        storageIndirectBuffers_;

    std::vector<DebugDataBuffer> debugDataBufferData_ = {DebugDataBuffer()};
};
