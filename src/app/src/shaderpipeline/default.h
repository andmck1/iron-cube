#pragma once

#include "core_render/vulkan/shaderpipeline.h"

class DefaultShaderPipeline : public VulkanShaderPipeline
{
    public:
    DefaultShaderPipeline();
};

class CubeShaderPipeline : public VulkanShaderPipeline
{
    public:
    CubeShaderPipeline();
};
