#include "buffertypes.h"

[[vk::binding(InstancedVF::BufferSlots::DEBUGDATA)]]
RWStructuredBuffer<InstancedVF::DebugDataBuffer> debugData;

[[vk::binding(InstancedVF::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk_push_constant]]
InstancedVF::Constants constants;

[shader("fragment")]
float4 main(InstancedVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);

    float4 colour = inVert.colour;
    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::TEXTURE:
    {
        colour = sampler.Sample(inVert.uv);
        break;
    }
    case FragmentColourType::PRIMITIVE:
    {
        break;
    }
    case FragmentColourType::NORMALS:
    {
        break;
    }
    case FragmentColourType::LOD:
    {
        break;
    }
    default:
        break;
    }
    return colour;
}
