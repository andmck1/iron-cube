#pragma once

#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/buffer.h"
#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct BufferBarrierInfo
{
    vk::Flags<vk::AccessFlagBits2> sourceAccess;
    vk::Flags<vk::AccessFlagBits2> destinationAccess;
    vk::Flags<vk::PipelineStageFlagBits2> sourceStage;
    vk::Flags<vk::PipelineStageFlagBits2> destinationStage;
    vk::raii::Buffer *buffer;
};

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    void addBuffer(BufferDescription description)
    {
        uint slotNumber = description.bufferSlot;
        if ((slotNumber + 1) > buffers_.size())
            buffers_.resize(slotNumber + 1);
        buffers_[slotNumber].init(description);
    }

    void addBufferWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        vk::DescriptorType bufferType, vk::raii::Buffer *buffer,
        vk::DeviceSize bufferSize,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );

    bool initDescriptorLayout();
    bool initDescriptorSets();
    bool createDescriptorPool();
    void shutdownBuffers();

    virtual void initBufferDescriptions() = 0;
    virtual void initBuffers() = 0;

    void addBarrier(BufferBarrierInfo barrierInfo);
    void setBarrier();

    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;

    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    std::vector<BufferDescription> bufferDescriptions_;
    std::vector<VulkanBuffer> buffers_;
    std::vector<vk::BufferMemoryBarrier2> barriers_;
};
