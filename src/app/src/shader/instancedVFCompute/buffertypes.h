// Instanced vertex/fragment with compute pipeline buffer types

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
        REORDERED_INSTANCES,
        LOD_STATS,
        INSTANCES,
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

    struct InstancedVertexInputType
    {
        // VertexInputType vertex;
        // Instance instance;
        float4 position;
        float4 tex;
        float4 normal;

        float4 instancePosition;
        float4 instanceRotation;
        float4 instanceScale;
        float4 lodLevel;
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
    };

    struct DebugDataBuffer
    {
        uint32_t isDebugQueryOn = 0;
        uint32_t nComputeShaderInvocations = 0;
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
