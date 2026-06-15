#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

// TODO - a lot shared between this and default mesh, can probably create a
// Vulkan Shader
class DefaultShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
};

class DefaultVulkanShaderPipeline : public VulkanShaderPipeline
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

    protected:
    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory_ = nullptr;
    vk::raii::Buffer vertexBuffer_ = nullptr;
    vk::raii::DeviceMemory indexBufferMemory_ = nullptr;
    vk::raii::Buffer indexBuffer_ = nullptr;
    vk::raii::QueryPool queryPool_ = nullptr;

    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    Geometry *geometry_ = nullptr;

    std::array<uint64_t, 1> pipelineStatistics_ = {0};
    bool isQuerying_ = false;

    bool initGraphicsPipeline();
    bool initPipeline();
    bool initVertexBuffer();
    bool initIndexBuffer();
    bool initDescriptorLayout();
    bool initUniformBuffers();
    bool initDescriptorSets();
    bool createDescriptorPool();
    bool createQueryPool();
};
