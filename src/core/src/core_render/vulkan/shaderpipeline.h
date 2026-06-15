#pragma once

#include "core_common/types.h"
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
    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    ShaderPipelineUpdateData *updateData_ = nullptr;

    // Descriptor infrastructure
    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    // Buffer infrastructure
    std::vector<std::vector<VulkanBuffer>> uniformBuffers_;
    std::vector<std::vector<VulkanBuffer>> storageBuffers_;
    std::vector<vk::ShaderStageFlags> uniformBufferStageFlags_;
    std::vector<vk::ShaderStageFlags> storageBufferStageFlags_;
    std::vector<bool> storageBufferIsReadWrite_;

    // Debug data (always the last storage buffer)
    std::vector<DebugDataBuffer> debugDataBufferData_ = {DebugDataBuffer()};

    // Template helpers for buffer registration
    template <typename DataType>
    void addUniformBuffer(
        vk::DeviceSize bufferSize, DataType *data,
        vk::BufferUsageFlagBits bufferType,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    )
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );
        uniformBuffers_.push_back({});
        uniformBufferStageFlags_.push_back(stageFlags);
        for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
        {
            VulkanBuffer newBuffer;
            newBuffer.init(bufferSize, data, bufferType, true, false);
            uniformBuffers_.back().emplace_back(std::move(newBuffer));
        }
    }

    template <typename DataType>
    void addStorageBuffer(
        vk::DeviceSize bufferSize, DataType *data,
        vk::BufferUsageFlagBits bufferType, bool isReadWrite,
        vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll
    )
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );
        storageBuffers_.push_back({});
        storageBufferStageFlags_.push_back(stageFlags);
        storageBufferIsReadWrite_.push_back(isReadWrite);
        for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
        {
            VulkanBuffer newBuffer;
            newBuffer.init(bufferSize, data, bufferType, false, isReadWrite);
            storageBuffers_.back().emplace_back(std::move(newBuffer));
        }
    }

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
