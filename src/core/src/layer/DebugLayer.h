#pragma once

#include "layer/DebugLayerData.h"
#include "layer/ILayer.h"
#include "scene/SceneData.h"
#include <chrono>
#include <vector>

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    void render() override;
    void shutdown() override;

    void setDebugData(DebugLayerData *debugData);

    private:
    DebugLayerData *debugData_;

    std::chrono::steady_clock::time_point lastTime_;
    float fps_ = 1.f;
    float frameTime_ = 1.f;

    // TODO - array?
    std::vector<float> frameTimes_ = {};
    static constexpr size_t MaxFrameTimes = 300;

    // TODO - get rid
    TriangleTransform transform_;
    bool showDemoWindow_ = true;
};
