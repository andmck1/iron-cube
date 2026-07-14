#pragma once

#include "shader/commontypes.h"

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define AS_GROUP_SIZE 32
#define MAX_LOD_COUNT 5

namespace InstancedVFComputeFlat
{
    enum BufferSlots : uint
    {
        MVP,
        SCENE,
        ANIMATION,
        DEBUG,
        INSTANCES,
        INDIRECT_COMMANDS,
        INDIRECT_DRAW_STATS,
        LOD,
        LAST
    };

    struct VertexOutputType
    {
        float4 position SEMANTIC(SV_Position);
        float4 colour;
    };

    struct MVPBuffer
    {
        float4x4 model;
        float4x4 view;
        float4x4 projection;
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

    struct FrustumData
    {
        FrustumPlane planes[6];
        float4 sphere;
        FrustumCone cone;
    };

    struct SceneBuffer
    {
        float4 eyePosition;
        FrustumData frustum;
        uint4 instanceCount;
        uint4 visibilityFunc;
        float4 minMeshBounds;
        float4 maxMeshBounds;
        float4 maxLODDistance;
        float4 lodExponent;
    };

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nComputeShaderInvocations = 0;
        uint32_t nVertexShaderInvocations = 0;
        uint32_t nFragmentShaderInvocations = 0;
    };

    struct IndirectDrawStats
    {
        uint4 lodCount[MAX_LOD_COUNT];
        uint32_t drawCount = 0;
    };

    struct LOD
    {
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
        uint32_t distance = 0;
        uint32_t pad0;
    };

    // Must match vk::DrawIndexedIndirectCommand layout exactly
    struct IndirectDrawCommand
    {
        uint indexCount;
        uint instanceCount;
        uint firstIndex;
        int vertexOffset;
        uint firstInstance;
    };
} // namespace InstancedVFComputeFlat
