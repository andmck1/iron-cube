/**
 * @file
 * @brief Scene implementation with procedural triangle generation.
 */

#pragma once

#include "scene/IScene.h"

/**
 * @brief Scene implementation that generates a centered, upward-pointing triangle.
 *
 * Stores vertex data in SceneData struct for cache-friendly access.
 * Implements IScene interface for service locator registration.
 */
class Scene : public IScene
{
    public:
    bool initialize() override;
    const std::vector<Vertex>& getVertices() const override;
    uint32_t getVertexCount() const override;
    bool isDirty() const override;
    void markClean() override;
    void shutdown() override;

    void setTriangleTransform(const TriangleTransform& transform) override;
    TriangleTransform getTriangleTransform() const override;
    bool hasTriangleTransformChanged() const override;
    void markTransformClean() override;

    private:
    void regenerateVertices();

    SceneData data_;
    TriangleTransform transform_;
    bool is_dirty_;
    bool transformDirty_;
};
