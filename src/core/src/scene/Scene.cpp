#include "scene/Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool Scene::initialize()
{
    regenerateVertices();
    is_dirty_ = true;

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

void Scene::setTriangleTransform(const TriangleTransform &transform)
{
    transform_ = transform;
    transformDirty_ = true;
    regenerateVertices();
    is_dirty_ = true;
}

TriangleTransform Scene::getTriangleTransform() const
{
    return transform_;
}

bool Scene::hasTriangleTransformChanged() const
{
    return transformDirty_;
}

void Scene::markTransformClean()
{
    transformDirty_ = false;
}

void Scene::regenerateVertices()
{
    data_.clear();

    // Base triangle vertices (centered, upward-pointing)
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

    for (const auto &base : baseVerts)
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
