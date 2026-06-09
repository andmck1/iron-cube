#pragma once

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

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

struct MVPBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

// Mental note - 32 bits in a float4 and must be in groups of 32 bits
struct ModelData
{
    uint32_t nVertices;
};

#endif
