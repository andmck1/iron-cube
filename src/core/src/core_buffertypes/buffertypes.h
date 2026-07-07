// uint32_t = 4 bytes
// float4 = 4 bytes
// float4x4 = 64 bytes
// constant buffer = 12 byte alignment

#pragma once

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define AS_GROUP_SIZE 32
#define MAX_LOD_COUNT 5

#define PI2 6.28318530719

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
using uint4 = glm::uvec4;
#endif

struct VertexInputType
{
    float4 position;
    float4 tex;
    float4 normal;
};

struct InstancedVertexInputType
{
    float4 position;
    float4 tex;
    float4 normal;

    float4 instancePosition;
    float4 instanceRotation;
    float4 instanceScale;
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

// Some help from chaoticbob
enum
{
    FRUSTUM_PLANE_LEFT = 0,
    FRUSTUM_PLANE_RIGHT = 1,
    FRUSTUM_PLANE_TOP = 2,
    FRUSTUM_PLANE_BOTTOM = 3,
    FRUSTUM_PLANE_NEAR = 4,
    FRUSTUM_PLANE_FAR = 5,
};

enum VisibilityFunc : uint
{
    VISIBILITY_FUNC_NONE = 0,
    VISIBILITY_FUNC_PLANES = 1,
    VISIBILITY_FUNC_SPHERE = 2,
    VISIBILITY_FUNC_CONE = 3,
    VISIBILITY_FUNC_CONE_AND_NEAR_PLANE = 4,
};

struct FrustumPlane
{
    float4 coefficients;
};

struct FrustumCone
{
    float3 tip;
    float height;
    float3 direction;
    float angle;
};

struct FrustumData
{
    FrustumPlane planes[6];
    float4 sphere;
    FrustumCone cone;
};

struct MVPBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct SceneBuffer
{
    float4 eyePosition;
    float4x4 view;
    float4x4 projection;
    FrustumData frustum;
    uint32_t instanceCount;
    uint32_t meshletCount;
    uint32_t visibilityFunc;
    uint32_t pad0;
    uint4 indexCounts[MAX_LOD_COUNT];
    uint4 indexOffsets[MAX_LOD_COUNT];
    uint4 vertexOffsets[MAX_LOD_COUNT];
    // TODO - parameterise this (compile time const)
    uint4 meshletLODOffsets[MAX_LOD_COUNT];
    uint4 meshletLODCounts[MAX_LOD_COUNT];
    float4 minMeshBounds;
    float4 maxMeshBounds;
    float4 maxLODDistance;
    float4 lodExponent;
    uint32_t isLODOn;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
};

struct TaskWorkBuffer
{
    uint32_t workStart;
    uint32_t workCount;
    uint32_t pad0;
};

struct AnimationBuffer
{
    uint32_t isAnimationOn;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    float4 rotationFrequency;
    float4 deltaTime;
    float4 currentTime;
};

struct DebugDataBuffer
{
    uint32_t isDebugQueryOn = 0;
    uint32_t nVertexShaderInvocations = 0;
    uint32_t nTaskShaderInvocations = 0;
    uint32_t nMeshShaderInvocations = 0;
    uint32_t nFragmentShaderInvocations = 0;
    uint32_t nTriangles = 0;
};

struct Instance
{
    float4 position;
    float4 rotation;
    float4 scale;
};

struct LODData
{
    uint32_t baseOffset;
    uint32_t slotCounter;
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
