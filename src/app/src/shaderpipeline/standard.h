#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    uint32_t *lodIndex = 0;
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
    bool initGraphicsPipeline();
    bool initPipeline();
    bool initIndexBuffer();
    bool initVertexBuffer();

    void initUniformBuffers() override;
    void initStorageBuffers() override;

    VulkanBufferVertex vertexBuffer_;
    VulkanBufferIndex indexBuffer_;
    DefaultShaderPipelineUpdateData *updateData_ = nullptr;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};

    uint32_t nDrawCalls_ = 0;
};
