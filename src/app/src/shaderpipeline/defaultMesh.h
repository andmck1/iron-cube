#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "core_render/vulkan/buffer.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultMeshShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
};

class DefaultVulkanMeshShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) override;
    bool update() override;
    bool bind() override;
    bool shutdown() override;
    bool recreateGraphicsPipeline() override;
    bool draw() override;
    PipelineStatistics getPipelineStatistics() override;
    bool updatePipelineStatistics() override;
    bool postRender() override;

    protected:
    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
    vk::raii::QueryPool queryPool_ = nullptr;

    std::vector<std::vector<VulkanBuffer>> uniformBuffers_;
    std::vector<std::vector<VulkanBuffer>> storageBuffers_;
    std::vector<vk::ShaderStageFlags> uniformBufferStageFlags_;
    std::vector<vk::ShaderStageFlags> storageBufferStageFlags_;
    std::vector<bool> storageBufferIsReadWrite_;

    Geometry *geometry_ = nullptr;
    std::vector<DebugDataBuffer> debugDataBufferData_ = {DebugDataBuffer()};

    bool isWaitingOnQueryPool_ = false;
    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    bool isQuerying_ = false;

    bool initGraphicsPipeline();
    bool initPipeline(Geometry *geometry);
    bool initDescriptorLayout();
    void initUniformBuffers();
    void initStorageBuffers();
    bool initDescriptorSets();
    bool createDescriptorPool();
    bool createQueryPool();

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

    void addBufferWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        vk::DescriptorType bufferType, vk::raii::Buffer *buffer,
        vk::DeviceSize bufferSize,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );
};
