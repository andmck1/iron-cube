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
    Scene *scene;

    // FPS tracking
    std::chrono::steady_clock::time_point lastTime_;
    float fps_;
    float frameTime_;
    std::vector<float> frameTimes_;
    static constexpr size_t MaxFrameTimes = 300;

    // Transform state (mirrors scene transform for UI binding)
    TriangleTransform transform_;
    bool showDemoWindow_;
};
