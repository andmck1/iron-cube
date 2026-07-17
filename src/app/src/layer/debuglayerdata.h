#pragma once

#include "core_common/helpers.h"
#include "shader/commontypes.h"
#include "core_common/timer.h"
#include "core_scene/scene.h"
#include <algorithm>
#include <vector>

static constexpr int DEBUG_HISTORY_FRAMES = 60;

struct PipelineTimeHistory
{
    float updateTime[DEBUG_HISTORY_FRAMES] = {0.f};
    float drawTime[DEBUG_HISTORY_FRAMES] = {0.f};
    float fenceWaitTime[DEBUG_HISTORY_FRAMES] = {0.f};
    float gpuPercent[DEBUG_HISTORY_FRAMES] = {0.f};
    int head_ = 0;

    void push(float updateMs, float drawMs, float fenceMs, float gpuPct)
    {
        updateTime[head_] = updateMs;
        drawTime[head_] = drawMs;
        fenceWaitTime[head_] = fenceMs;
        gpuPercent[head_] = gpuPct;
        head_ = (head_ + 1) % DEBUG_HISTORY_FRAMES;
    }

    float getMax(const float *data) const
    {
        return *std::max_element(data, data + DEBUG_HISTORY_FRAMES);
    }
};

class DebugLayerData
{
    public:
    DebugLayerData() = default;
    Scene *scene;
    Timer *timer;
    VisibilityFunc *visibilityFunc;
    bool *isAnimationOn;

    int *nInstances;
    float *discOuterRadius;
    float *discInnerRadius;
    float *discHeight;

    float *cameraFarPlane;
    bool *isDebugStatsOn;
    std::vector<GPUStats> *gpuStats;
    uint32_t *lodIndex;
    float *maxLODDistance;
    float *lodExponent;
    bool *isLODOn;
    std::vector<PipelineTimeHistory> timeHistories;
    FragmentColourType *fragmentColourType;
};
