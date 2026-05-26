#pragma once

#include "glm/ext/vector_float3.hpp"
#include <memory>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 colour;
};

struct Geometry
{
    std::shared_ptr<std::vector<Vertex>> vertices;
    uint32_t vertexCount;
};

struct Transform
{
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 rotation = {0.f, 0.f, 0.f};
    glm::vec3 scale = {1.f, 1.f, 1.f};
};
