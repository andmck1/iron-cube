#pragma once

#include "core_render/irenderer.h"
#include "core_layer/debuglayerdata.h"
#include "core_layer/ilayer.h"
#include "core_common/types.h"
#include <string>
#include <unordered_map>

class DebugLayer : public ILayer
{
    public:
    bool initialise() override;
    bool frame() override;

    void setDebugData(DebugLayerData *debugData);

    int shaderTypeUpdate = -1;

    protected:
    IRenderer *renderer_;
    DebugLayerData *debugData_;
    std::unordered_map<std::string, Transform> entityTransforms_;
    glm::vec3 cubeColor_ = {1.0f, 0.5f, 0.2f};
};
