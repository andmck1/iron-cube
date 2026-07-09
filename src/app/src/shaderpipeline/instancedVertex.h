#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "shader/instancedVF/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>

class InstancedVertexShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
    uint32_t *lodIndex = 0;
    bool *isAnimationOn = nullptr;
    float *rotationFrequency = nullptr;
    Timer *timer = nullptr;
};

class InstancedVertexVulkanShaderPipeline : public VulkanShaderPipeline
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
    bool initGraphicsPipeline();
    bool initPipeline();

    void initBuffers() override;

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;
    VulkanBuffer instanceBuffer_;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    InstancedVertexShaderPipelineUpdateData *updateData_ = nullptr;

    uint32_t nDrawCalls_ = 0;

    std::vector<InstancedVF::DebugDataBuffer> debugDataBufferData_ = {
        InstancedVF::DebugDataBuffer()
    };

    std::array<BufferDescription, InstancedVF::LAST> bufferDescriptions_ = {
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::MVP)
            .setBufferSize(sizeof(InstancedVF::MVPBuffer))
            .setData(static_cast<InstancedVF::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            ),
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedVF::AnimationBuffer))
            .setData(static_cast<InstancedVF::AnimationBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eVertex),

        StorageReadWrite.setBufferSlot(InstancedVF::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedVF::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    };
};
