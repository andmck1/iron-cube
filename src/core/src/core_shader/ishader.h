#pragma once

#include <vulkan/vulkan.h>

struct BaseShaderConfig
{
    virtual ~BaseShaderConfig() = default;
};

struct BaseBuffer
{
};

class IShader
{
    public:
    virtual ~IShader() = default;

    virtual void init(const BaseShaderConfig &shaderConfig) = 0;
    virtual void set(BaseBuffer buffer) = 0;
};
