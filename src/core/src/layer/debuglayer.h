#pragma once

#include "layer/debuglayerdata.h"
#include "layer/ilayer.h"
#include "common/types.h"
#include <string>
#include <unordered_map>

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    bool render() override;
    void shutdown() override;

    void setDebugData(DebugLayerData *debugData);

    private:
    DebugLayerData *debugData_;

    std::unordered_map<std::string, Transform> entityTransforms_;
    glm::vec3 cubeColor_ = {1.0f, 0.5f, 0.2f};
};
