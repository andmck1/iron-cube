#pragma once

#include "layer/ilayer.h"
#include "scene/scene.h"

class SceneLayer : public ILayer
{
    public:
    Scene scene = Scene();

    bool initialise() override;
    bool render() override;
    void shutdown() override;
};
