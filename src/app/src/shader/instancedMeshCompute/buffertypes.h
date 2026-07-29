#pragma once

#include "shader/commontypes.h"

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define TS_GROUP_SIZE 32
#define MAX_INDIRECT_COMMAND_COUNT 4096

namespace InstancedMeshCompute
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
        INDIRECT_COMMANDS,
        INDIRECT_DRAW_COMMANDS,
        LOD_INSTANCES,
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
        uint32_t maxGroupsPerDraw;
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
    };

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nComputeLODStatsInvocations = 0;
        uint32_t nComputeDrawCommandsInvocations = 0;
        uint32_t nTaskShaderInvocations = 0;
        uint32_t nMeshShaderInvocations = 0;
        uint32_t nFragmentShaderInvocations = 0;
        uint32_t nTriangles = 0;
        uint32_t _pad0;
    };

    struct MeshTasksIndirectCommand
    {
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
    };

    // Separate out count for command buffer and stat data
    struct IndirectCommandBuffer
    {
        uint32_t indirectDrawCommandCount;
        uint32_t visibleInstanceCount;
        uint32_t lodHistogram[MAX_LOD_COUNT];
        uint32_t meshletWorkOffsets[MAX_LOD_COUNT];
        uint32_t _pad0;
        uint32_t _pad1;
    };

    struct Constants
    {
        FragmentColourType fragmentColourType;
    };
} // namespace InstancedMeshCompute
