#pragma once

#include <vulkan/vulkan.h>

struct BaseShaderConfig
{
    virtual ~BaseShaderConfig() = default;
};

class IShader
{
    public:
    virtual ~IShader() = default;

    virtual void init(const BaseShaderConfig &shaderConfig) = 0;
    virtual VkPipelineShaderStageCreateInfo getShaderStageInfo() const = 0;
};
