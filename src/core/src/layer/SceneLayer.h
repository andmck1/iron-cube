#pragma once

#include "layer/ILayer.h"
#include "scene/Scene.h"

class SceneLayer : public ILayer
{
    public:
    Scene scene = Scene();

    bool initialise() override;
    bool render() override;
    void shutdown() override;
};
