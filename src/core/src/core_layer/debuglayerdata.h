#pragma once

#include "core_layer/idebuglayerdata.h"
#include "core_common/timer.h"
#include "core_scene/scene.h"

class DebugLayerData : public IDebugLayerData
{
    public:
    DebugLayerData() = default;
    Scene *scene;
    Timer *timer;
    int *visibilityFunc;
};
