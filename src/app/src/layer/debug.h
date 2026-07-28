#pragma once

#include "core_render/irenderer.h"
#include "layer/debuglayerdata.h"
#include "core_layer/ilayer.h"
#include <deque>

constexpr uint MAX_N_FRAME_TIMES = 60;
constexpr float FRAME_TIME_UPDATE_PERIOD = 1.f;

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    bool frame() override;
    bool update() override;
    bool postRender() override;
    bool end() override;

    void setDebugData(DebugLayerData *debugData);

    int shaderTypeUpdate = -1;
    int selectedShaderType = 0;

    protected:
    IRenderer *renderer_;
    DebugLayerData *debugData_;
    std::deque<float> frameTimes_;
    float timeSinceFrameTimeUpdate_ = 0.f;
    float averageFrameTime_ = 0.f;
    float stdFrameTime_ = 0.f;
    float maxFrameTime_ = 10.f;
};
