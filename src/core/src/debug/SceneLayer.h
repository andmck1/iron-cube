/**
 * @file
 * @brief Scene rendering layer that delegates to VulkanRenderer.
 */

#pragma once

#include "debug/ILayer.h"
#include "rendering/IRenderer.h"
#include "scene/Scene.h"

/**
 * @brief Scene layer that renders the 3D scene.
 *
 * Delegates rendering to VulkanRenderer::renderScene() which records
 * draw commands into the current command buffer.
 */
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
