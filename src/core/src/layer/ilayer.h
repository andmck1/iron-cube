#pragma once

#include "render/irenderer.h"

class ILayer
{
    public:
    virtual ~ILayer() = default;

    virtual bool initialise() = 0;
    virtual bool render() = 0;
    virtual void shutdown() = 0;

    protected:
    IRenderer *renderer_;
};
