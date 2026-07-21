#include "buffertypes.h"

[[vk::binding(SimpleVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<SimpleVF::DebugDataBuffer> debugData;

[[vk::binding(SimpleVF::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk_push_constant]]
SimpleVF::Constants constants;

[shader("fragment")]
float4 main(SimpleVF::VertexOutputType inVert) : SV_Target
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
    case FragmentColourType::LOD:
    {
        float lodColour = float(constants.lodLevel) + 1.0;
        colour = float4(
            lodColour / MAX_LOD_COUNT, lodColour / MAX_LOD_COUNT / 3.0,
            lodColour / MAX_LOD_COUNT / 5.0, 1.f
        );
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
    default:
    {
        colour = { 1.f, 1.f, 1.f, 1.f };
        break;
    }
    }

    return colour;
}
