#include "mesh/mesh.h"
#include "render/IRenderer.h"
#include "service/ServiceLocator.h"

void Mesh::setVertices(const std::vector<Vertex> &vertices)
{
    vertices_ = vertices;
}

bool Mesh::render()
{
    if (vertices_.empty())
        return false;

    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return false;

    renderer->uploadVertices(
        vertices_.data(), static_cast<uint32_t>(vertices_.size())
    );
    renderer->submitDraw(static_cast<uint32_t>(vertices_.size()));

    return true;
}
