#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "shader/instancedVFCompute/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>

class InstancedVertexComputeShaderPipelineUpdateData
    : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
    uint32_t *lodIndex = 0;
    VisibilityFunc *visibilityFunc = nullptr;
    float *maxLODDistance = nullptr;
    float *lodExponent = nullptr;
    bool *isLODOn = nullptr;
    bool *isAnimationOn = nullptr;
    float *rotationFrequency = nullptr;
    Timer *timer = nullptr;
};

class InstancedVertexComputeVulkanShaderPipeline : public VulkanShaderPipeline
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

    bool initGraphicsPipeline();
    bool initComputeShaderPipeline();
    bool initPipeline();
    bool initIndexBuffer();
    bool initVertexBuffer();
    bool initInstanceBuffer();

    void initUniformBuffers() override;
    void initStorageBuffers() override;

    std::optional<vk::raii::Pipeline> computePipeline_ = nullptr;

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;
    VulkanBuffer instanceBuffer_;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    InstancedVertexComputeShaderPipelineUpdateData *updateData_ = nullptr;

    uint32_t nDrawCalls_ = 0;

    std::vector<InstancedVFCompute::DebugDataBuffer> debugDataBufferData_ = {
        InstancedVFCompute::DebugDataBuffer()
    };
};
