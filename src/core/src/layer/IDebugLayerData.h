#pragma once

#include "scene/Scene.h"
class IDebugLayerData
{
    public:
    virtual ~IDebugLayerData() = default;

    Scene *scene;
};
