#pragma once

#include "core_layer/ilayer.h"
#include "core_scene/scene.h"

class SceneLayer : public ILayer
{
    public:
    Scene scene = Scene();

    bool initialise() override;
    bool render() override;
    void shutdown() override;
};
