#pragma once

#include "core_shader/ishader.h"
#include <vector>

struct BaseMaterialConfig
{
    virtual ~BaseMaterialConfig() = default;
};

class IMaterial
{
    public:
    virtual ~IMaterial() = default;

    virtual void set() = 0;
    virtual void addShader(IShader *) = 0;

    protected:
    BaseMaterialConfig config_;
    std::vector<IShader *> shaders;
};
