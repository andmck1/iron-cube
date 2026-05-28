#pragma once

#include "core_shader/ishader.h"

struct BaseMaterialConfig
{
    virtual ~BaseMaterialConfig() = default;
};

class IMaterial
{
    public:
    virtual ~IMaterial() = default;

    virtual bool init() = 0;
    virtual bool set() = 0;
    virtual void addShader(IShader *) = 0;
};
