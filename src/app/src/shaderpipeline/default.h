#pragma once

#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include "glm/ext/matrix_float4x4.hpp"
#include <vulkan/vulkan_raii.hpp>

class DefaultShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    glm::mat4 *modelMatrixPtr = nullptr;
    glm::mat4 *viewMatrixPtr = nullptr;
};

class DefaultVulkanShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) override;
    bool update() override;
    bool bind() override;
    bool shutdown() override;

    protected:
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory_ = nullptr;
    vk::raii::Buffer vertexBuffer_ = nullptr;
    vk::raii::DeviceMemory indexBufferMemory_ = nullptr;
    vk::raii::Buffer indexBuffer_ = nullptr;

    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    bool initGraphicsPipeline();
    bool initPipeline(Geometry *geometry);
    bool initVertexBuffer(Geometry *geometry);
    bool initIndexBuffer(Geometry *geometry);
    bool initDescriptorLayout();
    bool initUniformBuffers();
    bool initDescriptorSets();
};
