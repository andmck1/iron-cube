#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class InstancedMeshShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
    int *visibilityFunc = nullptr;
    float *maxLODDistance = nullptr;
    float *lodExponent = nullptr;
    bool *isLODOn = nullptr;
};

class InstancedMeshVulkanShaderPipeline : public VulkanShaderPipeline
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

    uint sceneDataUniformBufferIndex_ = 0;
    uint workUniformBufferIndex_ = 0;
    uint debugDataStorageRWBufferIndex_ = 0;
    uint instancesStorageRBufferIndex_ = 0;

    bool initGraphicsPipeline();
    bool initPipeline(Geometry *geometry);

    void initUniformBuffers() override;
    void initStorageBuffers() override;

    InstancedMeshShaderPipelineUpdateData *updateData_ = nullptr;
    uint32_t maxTaskShaderThreadCount_;
};
