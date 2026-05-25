#pragma once

#include "common/timer.h"
#include "scene/scene.h"

class IDebugLayerData
{
    public:
    virtual ~IDebugLayerData() = default;

    Scene *scene;
    Timer timer;
};
