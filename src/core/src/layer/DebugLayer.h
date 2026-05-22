#pragma once

#include "glm/ext/vector_float3.hpp"
#include "layer/DebugLayerData.h"
#include "layer/ILayer.h"
#include <chrono>
#include <vector>

struct TransformData
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    float scale = 1.0f;
};

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

    bool showDemoWindow_ = true;

    TransformData transform_;
};
