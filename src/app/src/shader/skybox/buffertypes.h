#pragma once

#include "shader/commontypes.h"
namespace SkyboxBufferTypes
{
    enum BufferSlots : uint
    {
        MVP,
        TEXTURE,
        LAST
    };
    struct VertexOutputType
    {
        float4 position SEMANTIC(SV_Position);
        float3 direction;
    };
    struct MVPBuffer
    {
        float4x4 model;
        float4x4 view;
        float4x4 projection;
    };
} // namespace SkyboxBufferTypes
