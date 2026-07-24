#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(InstancedVFCompute::BufferSlots::MVP)]]
ConstantBuffer<InstancedVFCompute::MVPBuffer> mvp;

[[vk::binding(InstancedVFCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedVFCompute::AnimationBuffer> animation;

[[vk::binding(InstancedVFCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedVFCompute::BufferSlots::REORDERED_INSTANCES)]]
RWStructuredBuffer<InstancedVFCompute::VFCInstance> reorderedInstances;

[[vk_push_constant]]
InstancedVFCompute::Constants constants;

[shader("vertex")]
InstancedVFCompute::VertexOutputType
main(VertexInputType input, uint instanceID: SV_InstanceID)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    InstancedVFCompute::VertexOutputType output;

    float4x4 worldMatrix = reorderedInstances[instanceID].worldMatrix;
    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, worldMatrix));
    output.position = mul(mvpMatrix, input.position);
    output.uv = input.tex.xy;

    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::LOD:
    {
        uint lodLevel = reorderedInstances[instanceID].lodLevel;
        float lodColour = float(lodLevel) + 1.0;
        output.colour = float4(
            lodColour / MAX_LOD_COUNT, lodColour / MAX_LOD_COUNT / 3.0,
            lodColour / MAX_LOD_COUNT / 5.0, 1.f
        );
        break;
    }
    case FragmentColourType::NORMALS:
    {
        output.colour = (input.normal + 1.f) / 2.f;
        output.colour.w = 1.f;
        break;
    }
    case FragmentColourType::PRIMITIVE:
    {
        break;
    }
    default:
    {
        output.colour = float4(1.f, 1.f, 1.f, 1.f);
        break;
    }
    }

    return output;
}
