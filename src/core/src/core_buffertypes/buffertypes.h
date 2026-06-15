#pragma once

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define AS_GROUP_SIZE 32

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#ifdef __SLANG__
#define SEMANTIC(x) : x
#else
#include <cstdint>
#include <glm/glm.hpp>
#define SEMANTIC(x)

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;
#endif

struct VertexInputType
{
    float4 position;
    float4 tex;
    float4 normal;
};

struct VertexOutputType
{
    float4 position SEMANTIC(SV_Position);
    float4 colour;
};

struct MeshletTriangle
{
    uint8_t vertexIndex0;
    uint8_t vertexIndex1;
    uint8_t vertexIndex2;
};

struct Meshlet
{
    // These are indices to positions in Mesh (Geometry)
    int localVertices[N_MAX_VERTICES];

    // These are indices for vertices
    MeshletTriangle triangles[N_MAX_TRIANGLES];
};

struct OptimisedMeshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
};

struct MVPBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct SceneBuffer
{
    float4x4 view;
    float4x4 projection;
    uint instanceCount;
    uint meshletCount;
};

struct DebugDataBuffer
{
    uint32_t n = 5;
};

struct Instance
{
    float4x4 transform = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}
    };
};

// Mental note - 32 bits in a float4 and must be in groups of 32 bits
struct ModelData
{
    uint32_t nVertices;
};

// TODO - this name sucks
struct Payload
{
    uint instanceIndices[AS_GROUP_SIZE];
    uint meshletIndices[AS_GROUP_SIZE];
};

#endif
