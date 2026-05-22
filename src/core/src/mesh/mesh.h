#pragma once

#include "render/IRenderer.h"
#include <vector>

class Mesh
{
    public:
    void setVertices(const std::vector<Vertex> &vertices);
    bool render();

    private:
    std::vector<Vertex> vertices_ = {};
};
