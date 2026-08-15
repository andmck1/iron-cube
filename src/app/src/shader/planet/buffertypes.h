#pragma once

#include "shader/commontypes.h"
namespace PlanetBufferTypes
{
    enum BufferSlots : uint
    {
        SCENE,
        MVP,
        TEXTURE,
        LAST
    };

    struct VertexOutputType
    {
        float4 position SEMANTIC(SV_Position);
        float2 uv;
        float4 colour;
        float3 rayOrigin;
        float3 rayDir;
        float3 sphereCentre;
        float sphereRadius;
    };

    struct FragmentOutputType
    {
        float4 colour SEMANTIC(SV_Target);
        float depth SEMANTIC(SV_Depth);
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
        DirectionalLight directionalLight;
    };
} // namespace PlanetBufferTypes
