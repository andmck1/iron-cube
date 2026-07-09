// Simple vertex/fragment pipeline buffer types

#pragma once

#include "shader/commontypes.h"

namespace SimpleVF
{
    enum BufferSlots : uint
    {
        MVP = 0,
        ANIMATION = 1,
        DEBUG = 2,
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
        uint32_t nFragmentShaderInvocations = 0;
    };

} // namespace SimpleVF
