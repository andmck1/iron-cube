#include "buffertypes.h"

[[vk::binding(InstancedVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVF::DebugDataBuffer> debugData;

[[vk::binding(InstancedVF::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk_push_constant]]
InstancedVF::Constants constants;

[shader("fragment")]
float4 main(InstancedVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);

    float4 colour;
    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::TEXTURE:
    {
        colour = sampler.Sample(inVert.uv);
        break;
    }
    case FragmentColourType::LOD:
    {
        colour = inVert.colour;
        break;
    }
    case FragmentColourType::PRIMITIVE:
    {
        break;
    }
    default:
    {
        colour = { 1.f, 1.f, 1.f, 1.f };
        break;
    }
    }

    return colour;
}
