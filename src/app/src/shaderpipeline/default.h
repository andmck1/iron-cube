#pragma once

#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultVulkanShaderPipeline : public VulkanShaderPipeline
{
    public:
    DefaultVulkanShaderPipeline();

    protected:
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;
};
