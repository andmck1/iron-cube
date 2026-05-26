#pragma once

#include "core_common/timer.h"
#include "core_scene/scene.h"

class IDebugLayerData
{
    public:
    virtual ~IDebugLayerData() = default;

    Scene *scene;
    Timer timer;
};
