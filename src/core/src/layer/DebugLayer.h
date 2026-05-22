#pragma once

#include "layer/DebugLayerData.h"
#include "layer/ILayer.h"
#include "helpers/data.h"

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    void render() override;
    void shutdown() override;

    void setDebugData(DebugLayerData *debugData);

    private:
    DebugLayerData *debugData_;

    bool showDemoWindow_ = true;

    Transform triangleTransform_;
};
