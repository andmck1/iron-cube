/**
 * @file
 * @brief Scene rendering layer that delegates to VulkanRenderer.
 */

#pragma once

#include "debug/ILayer.h"

class VulkanRenderer;

/**
 * @brief Scene layer that renders the 3D scene.
 *
 * Delegates rendering to VulkanRenderer::renderScene() which records
 * draw commands into the current command buffer.
 */
class SceneLayer : public ILayer
{
    public:
    explicit SceneLayer(VulkanRenderer* renderer);

    bool initialize() override;
    void render() override;
    void shutdown() override;

    private:
    VulkanRenderer* renderer_;
};
