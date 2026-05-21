#pragma once

#include "scene/SceneData.h"
#include <vector>
#include <cstdint>

class IScene
{
    public:
    virtual ~IScene() = default;

    virtual bool initialise() = 0;
    virtual const std::vector<Vertex> &getVertices() const = 0;
    virtual uint32_t getVertexCount() const = 0;
    virtual void shutdown() = 0;
    virtual TriangleTransform getTriangleTransform() const = 0;
    virtual void setTriangleTransform(const TriangleTransform &transform) = 0;
};
