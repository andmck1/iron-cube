#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct TriangleTransform
{
    glm::vec3 position;
    glm::vec3 rotation;
    float scale;

    TriangleTransform()
        : position{0.0f, 0.0f, 0.0f}, rotation{0.0f, 0.0f, 0.0f}, scale(1.0f)
    {
    }
};

struct SceneData
{
    std::vector<Vertex> vertices;
    uint32_t vertex_count;
    bool is_dirty;

    SceneData() : vertex_count(0), is_dirty(true)
    {
    }

    bool isValid() const
    {
        return vertex_count == static_cast<uint32_t>(vertices.size()) &&
               vertex_count > 0;
    }

    void clear()
    {
        vertices.clear();
        vertex_count = 0;
        is_dirty = false;
    }
};
