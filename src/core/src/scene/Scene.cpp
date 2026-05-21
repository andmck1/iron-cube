#include "scene/Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool Scene::initialise()
{
    regenerateVertices();

    return data_.isValid();
}

const std::vector<Vertex> &Scene::getVertices() const
{
    return data_.vertices;
}

uint32_t Scene::getVertexCount() const
{
    return data_.vertex_count;
}

void Scene::shutdown()
{
    data_.clear();
}

void Scene::setTriangleTransform(const TriangleTransform &transform)
{
    transform_ = transform;
    regenerateVertices();
}

TriangleTransform Scene::getTriangleTransform() const
{
    return transform_;
}

void Scene::regenerateVertices()
{
    data_.clear();

    // Base triangle vertices (centred, upward-pointing)
    glm::vec3 baseVerts[3] = {
        glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(0.0f, 0.5f, 0.0f)
    };

    // Build model matrix: scale first, then rotation, then translation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, transform_.position);
    model = glm::rotate(
        model, glm::radians(transform_.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)
    );
    model = glm::rotate(
        model, glm::radians(transform_.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)
    );
    model = glm::rotate(
        model, glm::radians(transform_.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)
    );
    model = glm::scale(model, glm::vec3(transform_.scale));

    for (const glm::vec3 &base : baseVerts)
    {
        glm::vec4 transformed = model * glm::vec4(base, 1.0f);
        data_.vertices.push_back(
            Vertex{
                {transformed.x, transformed.y, transformed.z},
                {1.0f, 0.0f, 0.0f}
            }
        );
    }
    data_.vertex_count = static_cast<uint32_t>(data_.vertices.size());
}
