#pragma once

#include "glm/ext/vector_float3.hpp"
#include "core_buffertypes/buffertypes.h"
#include "meshoptimizer.h"
#include <vector>

struct MeshBounds
{
    glm::vec3 minCorner;
    glm::vec3 maxCorner;

    float getWidth() const
    {
        return maxCorner.x - minCorner.x;
    }
    float getHeight() const
    {
        return maxCorner.y - minCorner.y;
    }
    float getDepth() const
    {
        return maxCorner.z - minCorner.z;
    }
};

struct Geometry
{
    std::vector<VertexInputType> vertices;
    std::vector<uint16_t> indices;
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletVertices;
    std::vector<uint8_t> meshletTriangles;
    std::vector<uint32_t> meshletTrianglesU32;
    MeshBounds meshBounds;
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
