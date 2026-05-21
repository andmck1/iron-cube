#pragma once

#include "scene/IScene.h"

class Scene : public IScene
{
    public:
    bool initialise() override;
    const std::vector<Vertex> &getVertices() const override;
    uint32_t getVertexCount() const override;
    void shutdown() override;

    void setTriangleTransform(const TriangleTransform &transform) override;
    TriangleTransform getTriangleTransform() const override;

    private:
    void regenerateVertices();

    SceneData data_;
    TriangleTransform transform_;
};
