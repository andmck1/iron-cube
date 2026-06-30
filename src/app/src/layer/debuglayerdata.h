#pragma once

#include "core_common/timer.h"
#include "core_scene/scene.h"

class DebugLayerData
{
    public:
    DebugLayerData() = default;
    Scene *scene;
    Timer *timer;
    int *visibilityFunc;
    bool *isAnimationOn;
    int *nInstanceRows;
    int *nInstanceColumns;
    int *nInstanceLayers;
    float *cameraFarPlane;
};
