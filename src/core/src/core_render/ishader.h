#pragma once

struct BaseShaderConfig
{
    virtual ~BaseShaderConfig() = default;
};

class IShader
{
    public:
    virtual ~IShader() = default;
};
