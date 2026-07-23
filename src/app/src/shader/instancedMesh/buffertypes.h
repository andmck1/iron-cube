// Instanced mesh shader pipeline buffer types

#pragma once

#include "shader/commontypes.h"

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define TS_GROUP_SIZE 32

namespace InstancedMesh
{
    enum BufferSlots : uint
    {
        SCENE,
        ANIMATION,
        DEBUG,
        MODEL_VERTICES,
        MESHLETS,
        VERTEX_INDICES,
        TRIANGLE_INDICES,
        INSTANCES,
        MESHLET_BOUNDING_SPHERES,
        TEXTURE,
        LAST
    };

    struct VertexOutputType
    {
        float4 position SEMANTIC(SV_Position);
        float4 colour;
        float2 uv;
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

    struct MeshletTriangle
    {
        uint8_t vertexIndex0;
        uint8_t vertexIndex1;
        uint8_t vertexIndex2;
    };

    struct Meshlet
    {
        int localVertices[N_MAX_VERTICES];
        MeshletTriangle triangles[N_MAX_TRIANGLES];
    };

    struct OptimisedMeshlet
    {
        uint vertexOffset;
        uint triangleOffset;
        uint vertexCount;
        uint triangleCount;
    };

    struct FrustumData
    {
        FrustumPlane planes[6];
        float4 sphere;
        FrustumCone cone;
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

    struct Payload
    {
        uint instanceIndices[TS_GROUP_SIZE];
        uint meshletIndices[TS_GROUP_SIZE];
        float lodLevel[TS_GROUP_SIZE];
    };

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nTaskShaderInvocations = 0;
        uint32_t nMeshShaderInvocations = 0;
        uint32_t nFragmentShaderInvocations = 0;
        uint32_t nTriangles = 0;
    };

    struct Constants
    {
        uint32_t workStart;
        uint32_t workCount;
        FragmentColourType fragmentColourType;
    };
} // namespace InstancedMesh
