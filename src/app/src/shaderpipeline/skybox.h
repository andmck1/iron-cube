#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class SkyboxShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    Cubemap *cubemap = nullptr;
    DirectionalLight *directionalLight = nullptr;
};

class SkyboxVulkanShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) override;
    bool initPipeline() override;
    bool update() override;
    bool bind() override;
    bool shutdown() override;
    bool recreateGraphicsPipeline() override;
    bool draw() override;
    PipelineStatistics getPipelineStatistics() override;
    bool updatePipelineStatistics() override;
    bool postRender() override;

    protected:
    void initBufferDescriptions() override;
    void initBuffers() override;
    bool initGraphicsPipeline();

    VulkanBuffer vertexBuffer_;
    VulkanBuffer indexBuffer_;
    SkyboxShaderPipelineUpdateData *updateData_ = nullptr;

    Geometry *geometry_ = nullptr;
};
