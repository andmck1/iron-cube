#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "shader/instancedMeshCompute/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>

class InstancedMeshComputeShaderPipelineUpdateData
    : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
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

class InstancedMeshComputeVulkanShaderPipeline : public VulkanShaderPipeline
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
    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};

    bool initGraphicsPipeline();
    bool initPipeline(Geometry *geometry);
    void initBufferDescriptions() override;

    void initBuffers() override;

    InstancedMeshComputeShaderPipelineUpdateData *updateData_ = nullptr;
    uint32_t maxTaskShaderThreadCount_;
    uint32_t nDrawCalls_ = 0;

    std::vector<InstancedMeshCompute::DebugDataBuffer> debugDataBufferData_ = {
        InstancedMeshCompute::DebugDataBuffer()
    };

    InstancedMeshCompute::Constants constants;
};
