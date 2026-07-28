#pragma once

#include "shader/commontypes.h"

#define THREAD_GROUP_SIZE 256

namespace InstancedVFCompute
{
    enum BufferSlots : uint
    {
        MVP,
        SCENE,
        ANIMATION,
        DEBUG,
        INSTANCES,
        CALCULATED_INSTANCES,
        REORDERED_INSTANCES,
        LOD_STATS,
        INDIRECT_COMMANDS,
        TEXTURE,
        LAST
    };

    struct VertexOutputType
    {
        float4 position SEMANTIC(SV_Position);
        float4 colour;
        float2 uv;
    };

    struct MVPBuffer
    {
        float4x4 model;
        float4x4 view;
        float4x4 projection;
    };

    struct CalculatedInstance
    {
        float4x4 worldMatrix;
        uint32_t lodLevel;
        uint32_t isVisible;
        uint32_t _pad0;
        uint32_t _pad1;
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
        Disc disc;
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
        uint32_t isLODOn;
        uint32_t pad1;
        uint32_t pad2;
        uint32_t pad3;
    };

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nComputeLODStatsInvocations = 0;
        uint32_t nComputeDrawCommandsInvocations = 0;
        uint32_t nVertexShaderInvocations = 0;
        uint32_t nFragmentShaderInvocations = 0;
    };

    struct Constants
    {
        FragmentColourType fragmentColourType;
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

    struct LODStats
    {
        uint32_t offset;
        uint32_t count;
        uint32_t _pad0;
        uint32_t _pad1;
    };
} // namespace InstancedVFCompute
