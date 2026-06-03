#pragma once

#include "core_common/types.h"

class IShaderPipeline
{
    public:
    virtual ~IShaderPipeline() = default;
    virtual bool init(Geometry *geometry) = 0;
    virtual bool bind() = 0;
    virtual bool shutdown() = 0;
};
