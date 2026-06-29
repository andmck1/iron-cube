#pragma once

#include "core_common/types.h"
#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/buffer.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    ShaderPipelineUpdateData *updateData_ = nullptr;

    // Descriptor infrastructure
    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    // Buffer infrastructure
    std::vector<std::vector<VulkanBufferUniform>> uniformBuffers_;
    std::vector<std::vector<VulkanBufferRStorage>> storageRBuffers_;
    std::vector<std::vector<VulkanBufferRWStorage>> storageRWBuffers_;

    // Debug data (always the last storage buffer)
    std::vector<DebugDataBuffer> debugDataBufferData_ = {DebugDataBuffer()};

    // Template helpers for buffer registration
    void addUniformBuffer(
        vk::DeviceSize bufferSize, void *data,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    );

    void addStorageRBuffer(
        vk::DeviceSize bufferSize, void *data,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    );

    void addStorageRWBuffer(
        vk::DeviceSize bufferSize, void *data,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    );

    // Shared descriptor methods
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

    // Pure virtual hooks for derived classes
    virtual void initUniformBuffers() = 0;
    virtual void initStorageBuffers() = 0;
};
