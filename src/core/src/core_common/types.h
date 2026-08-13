#pragma once
#include <stb_image.h>
#include <ktx.h>
#include "glm/ext/vector_float3.hpp"
#include "shader/commontypes.h"
#include "meshoptimizer.h"
#include <vector>
#include <vulkan/vulkan_raii.hpp>

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
    std::vector<uint32_t> indices;
    std::vector<meshopt_Meshlet> meshlets;
    std::vector<uint32_t> meshletVertices;
    std::vector<uint8_t> meshletTriangles;
    std::vector<uint32_t> meshletTrianglesU32;
    std::vector<glm::vec4> meshletBoundingSpheres;

    std::vector<uint32_t> vertexOffsets;
    std::vector<uint32_t> vertexCounts;
    std::vector<uint32_t> indexOffsets;
    std::vector<uint32_t> indexCounts;
    std::vector<uint32_t> meshletLODOffsets;
    std::vector<uint32_t> meshletLODCounts;
    MeshBounds meshBounds;
};

struct Transform
{
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 rotation = {0.f, 0.f, 0.f};
    glm::vec3 scale = {1.f, 1.f, 1.f};
};

struct Texture
{
    stbi_uc *pixels = nullptr;
    uint imageSize = 0;
    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;
};

struct Cubemap
{
    ktxTexture *texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 0;
    vk::Format format = vk::Format::eUndefined;
};

struct ShaderPipelineUpdateData
{
    virtual ~ShaderPipelineUpdateData() = default;
};
