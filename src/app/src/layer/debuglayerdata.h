#pragma once

#include "core_common/helpers.h"
#include "core_common/timer.h"
#include "core_scene/scene.h"
#include <vector>

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
    bool *isDebugStatsOn;
    std::vector<GPUStats> *gpuStats;
    unsigned int *lodIndex;
};
