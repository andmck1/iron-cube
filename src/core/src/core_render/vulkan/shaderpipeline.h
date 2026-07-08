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
    void addBuffer(
        BufferDescription description, vk::DeviceSize bufferSize, void *data
    )
    {
        description.bufferSlot = static_cast<uint>(bufferDescriptions_.size());
        bufferDescriptions_.push_back(description);
        buffers_.emplace_back();
        buffers_.back().init(description, bufferSize, data);
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

    virtual void initUniformBuffers() = 0;
    virtual void initStorageBuffers() = 0;

    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;

    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    std::vector<BufferDescription> bufferDescriptions_;
    std::vector<VulkanBuffer> buffers_;
};
