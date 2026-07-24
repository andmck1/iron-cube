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
    Texture *texture = nullptr;
    FragmentColourType *fragmentColourType = nullptr;
    float *discOuterRadius = nullptr;
    float *discInnerRadius = nullptr;
    float *discHeight = nullptr;
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
    bool initPipeline() override;
    void initBufferDescriptions() override;

    void initBuffers() override;

    void updateSceneData();
    void updateMVPData();
    void updateAnimationData();
    void updateInstancesData();
    void updateDebugData();

    void resetIndirectDrawCommands();
    void resetLODStats();
    void bindComputePipeline(uint pass);
    void lodStatsComputePass(uint dispatchCount);
    void drawCommandComputePass(uint dispatchCount);

    std::vector<std::optional<vk::raii::Pipeline>> computePasses_;

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    InstancedVertexComputeShaderPipelineUpdateData *updateData_ = nullptr;

    uint32_t nDrawCalls_ = 0;

    std::vector<InstancedVFCompute::DebugDataBuffer> debugDataBufferData_ = {
        InstancedVFCompute::DebugDataBuffer()
    };
};
