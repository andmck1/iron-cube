#pragma once

#include <vulkan/vulkan.h>

class IShader
{
    public:
    virtual ~IShader() = default;

    virtual VkPipelineShaderStageCreateInfo getShaderStageInfo() const = 0;
};
