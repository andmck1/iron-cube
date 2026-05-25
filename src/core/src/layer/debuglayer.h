#pragma once

#include "layer/debuglayerdata.h"
#include "layer/ilayer.h"
#include "common/types.h"

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    bool render() override;
    void shutdown() override;

    void setDebugData(DebugLayerData *debugData);

    private:
    DebugLayerData *debugData_;

    Transform triangleTransform_;
};
