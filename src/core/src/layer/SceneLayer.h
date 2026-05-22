#pragma once

#include "layer/ILayer.h"
#include "render/IRenderer.h"
#include "scene/Scene.h"

class SceneLayer : public ILayer
{
    public:
    Scene scene = Scene();

    bool initialise() override;
    void render() override;
    void shutdown() override;

    private:
    IRenderer *renderer_;
};
