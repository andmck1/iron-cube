#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "shader/simpleVF/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    uint32_t *lodIndex = nullptr;
    uint32_t *nFramesSinceStart = nullptr;
    bool *isAnimationOn = nullptr;
    float *rotationFrequency = nullptr;
    Timer *timer = nullptr;
};

class StandardVulkanShaderPipeline : public VulkanShaderPipeline
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
    void initBuffers() override;

    bool initGraphicsPipeline();
    bool initPipeline();

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;
    DefaultShaderPipelineUpdateData *updateData_ = nullptr;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};

    uint32_t nDrawCalls_ = 0;

    std::vector<SimpleVF::DebugDataBuffer> debugDataBufferData_ = {
        SimpleVF::DebugDataBuffer()
    };

    std::array<BufferDescription, SimpleVF::LAST> bufferDescriptions_ = {
        UniformBuffer.setBufferSlot(SimpleVF::BufferSlots::MVP)
            .setBufferSize(sizeof(SimpleVF::MVPBuffer))
            .setData(static_cast<SimpleVF::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            ),
        UniformBuffer.setBufferSlot(SimpleVF::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(SimpleVF::AnimationBuffer))
            .setData(static_cast<SimpleVF::AnimationBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eVertex),

        StorageReadWrite.setBufferSlot(SimpleVF::BufferSlots::DEBUG)
            .setBufferSize(sizeof(SimpleVF::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    };
};
