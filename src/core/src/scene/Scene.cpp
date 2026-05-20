#include "scene/Scene.h"
#include <glm/glm.hpp>

bool Scene::initialize()
{
    data_.clear();

    data_.vertices.push_back(Vertex{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}});
    data_.vertices.push_back(Vertex{{ 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}});
    data_.vertices.push_back(Vertex{{ 0.0f,  0.5f}, {1.0f, 0.0f, 0.0f}});
    data_.vertex_count = static_cast<uint32_t>(data_.vertices.size());
    is_dirty_ = true;

    return data_.isValid();
}

const std::vector<Vertex>& Scene::getVertices() const
{
    return data_.vertices;
}

uint32_t Scene::getVertexCount() const
{
    return data_.vertex_count;
}

bool Scene::isDirty() const
{
    return is_dirty_;
}

void Scene::markClean()
{
    is_dirty_ = false;
}

void Scene::shutdown()
{
    data_.clear();
    is_dirty_ = false;
}
