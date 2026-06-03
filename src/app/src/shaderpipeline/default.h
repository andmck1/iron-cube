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
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
    vk::raii::DeviceMemory vertexBufferMemory_ = nullptr;
    vk::raii::Buffer vertexBuffer_ = nullptr;
    bool initGraphicsPipeline();
    bool initVertexBuffer(Geometry *geometry);
};
