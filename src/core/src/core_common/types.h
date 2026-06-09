#pragma once

#include "glm/ext/vector_float3.hpp"
#include "core_buffertypes/buffertypes.h"
#include "meshoptimizer.h"
#include <vector>

struct Geometry
{
    std::vector<VertexInputType> vertices;
    std::vector<uint16_t> indices;
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletVertices;
    std::vector<uint8_t> meshletTriangles;
};

struct Transform
{
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 rotation = {0.f, 0.f, 0.f};
    glm::vec3 scale = {1.f, 1.f, 1.f};
};

struct ShaderPipelineUpdateData
{
    virtual ~ShaderPipelineUpdateData() = default;
};
