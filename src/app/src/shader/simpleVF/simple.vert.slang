#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(SimpleVF::BufferSlots::MVP)]]
ConstantBuffer<SimpleVF::MVPBuffer> mvp;

[[vk::binding(SimpleVF::BufferSlots::ANIMATION)]]
ConstantBuffer<SimpleVF::AnimationBuffer> animation;

[[vk::binding(SimpleVF::BufferSlots::DEBUGDATA)]]
RWStructuredBuffer<SimpleVF::DebugDataBuffer> debugData;

[[vk_push_constant]]
SimpleVF::Constants constants;

[shader("vertex")]
SimpleVF::VertexOutputType main(VertexInputType input)
{
    // In theory you can compile this out using constants
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    SimpleVF::VertexOutputType output;
    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::NORMALS:
    {
        output.colour = (input.normal + 1.f) / 2.f;
        output.colour.w = 1.f;
    }
    default:
        break;
    }

    output.position =
        mul(mvp.projection, mul(mvp.view, mul(mvp.model, input.position)));
    output.uv = input.tex.xy;
    return output;
}
