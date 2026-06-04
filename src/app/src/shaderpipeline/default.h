#pragma once

#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultVulkanShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool init(Geometry *geometry) override;
    bool bind() override;
    bool shutdown() override;

    protected:
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory_ = nullptr;
    vk::raii::Buffer vertexBuffer_ = nullptr;
    vk::raii::DeviceMemory indexBufferMemory_ = nullptr;
    vk::raii::Buffer indexBuffer_ = nullptr;
    uint32_t indexCount_ = 0;
    bool initGraphicsPipeline();
    bool initVertexBuffer(Geometry *geometry);
    bool initIndexBuffer(Geometry *geometry);
    bool initDescriptorLayout();
};
