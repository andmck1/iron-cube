#pragma once

#include "core_buffer/ibuffer.h"
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
    virtual IBuffer *addBuffer(IBuffer *buffer) = 0;
    virtual void set() = 0;
};
