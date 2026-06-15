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
    vk::raii::DeviceMemory vertexBufferMemory_ = nullptr;
    vk::raii::Buffer vertexBuffer_ = nullptr;
    vk::raii::DeviceMemory indexBufferMemory_ = nullptr;
    vk::raii::Buffer indexBuffer_ = nullptr;
    vk::raii::QueryPool queryPool_ = nullptr;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatisticsRaw_ = {0};
    PipelineStatistics pipelineStatistics_{};
    bool isQuerying_ = false;

    bool initGraphicsPipeline();
    bool initPipeline();
    bool initVertexBuffer();
    bool initIndexBuffer();
    bool createQueryPool();

    // Pure virtual hooks from base class
    void initUniformBuffers() override;
    void initStorageBuffers() override;
};
