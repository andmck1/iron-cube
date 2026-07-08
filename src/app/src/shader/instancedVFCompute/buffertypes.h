// Instanced vertex/fragment with compute pipeline buffer types

#pragma once

#include "shader/commontypes.h"

#define N_MAX_TRIANGLES 124
#define N_MAX_VERTICES 64

#define AS_GROUP_SIZE 32
#define MAX_LOD_COUNT 5

namespace InstancedVFCompute
{
    enum BufferSlots : uint
    {
        MVP = 0,
        SCENE = 1,
        ANIMATION = 2,
        DEBUG = 3,
        REORDERED_INSTANCES = 4,
        LOD_BASE_OFFSETS = 5,
        LOD_SLOT_COUNTERS = 6,
        INSTANCES = 7,
        INDIRECT_COMMANDS = 8
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

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nComputeShaderInvocations = 0;
        uint32_t nVertexShaderInvocations = 0;
        uint32_t nFragmentShaderInvocations = 0;
    };
} // namespace instancedVFCompute
