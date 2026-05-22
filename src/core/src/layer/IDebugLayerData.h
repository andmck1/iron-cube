#pragma once

#include "helpers/timer.h"
#include "scene/Scene.h"
class IDebugLayerData
{
    public:
    virtual ~IDebugLayerData() = default;

    Scene *scene;
    Timer timer;
};
