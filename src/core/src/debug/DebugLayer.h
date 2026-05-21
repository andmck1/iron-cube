#pragma once

#include "debug/ILayer.h"
#include "scene/Scene.h"
#include "scene/SceneData.h"
#include <chrono>
#include <vector>

class DebugLayer : public ILayer
{
    public:
    DebugLayer(Scene *scene);

    bool initialise() override;
    void render() override;
    void shutdown() override;

    private:
    Scene *scene_;

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
