#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(InstancedVF::BufferSlots::MVP)]]
ConstantBuffer<InstancedVF::MVPBuffer> mvp;

[[vk::binding(InstancedVF::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedVF::AnimationBuffer> animation;

[[vk::binding(InstancedVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVF::DebugDataBuffer> debugData;

[[vk_push_constant]]
InstancedVF::Constants constants;

[shader("vertex")]
InstancedVF::VertexOutputType main(InstancedVF::InstancedVertexInputType input)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    InstancedVF::VertexOutputType output;

    float4x4 worldMatrix = calculateModelMatrix(
        input.instancePosition, input.instanceRotation, input.instanceScale
    );

    float4 modelPosition = input.position;
    if (animation.isAnimationOn > 0)
    {
        float rotationAngle =
            PI2 * animation.rotationFrequency.x * animation.currentTime.x;
        modelPosition = {
            rotateVectorWithQuaternion(
                modelPosition.xyz,
                quaternionFromAxisAngle(float3(0.f, 1.f, 0.f), rotationAngle)
            ),
            1.f
        };
    }

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, worldMatrix));
    output.position = mul(mvpMatrix, modelPosition);
    output.uv = input.tex.xy;

    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::TEXTURE:
    {
        output.colour = float4(1.f, 1.f, 1.f, 1.f);
        break;
    }
    case FragmentColourType::LOD:
    {
        output.colour = float4(
            1.f / MAX_LOD_COUNT, 1.f / MAX_LOD_COUNT / 3.0,
            1.f / MAX_LOD_COUNT / 5.0, 1.f
        );
        break;
    }
    case FragmentColourType::NORMALS:
    {
        output.colour = float4(input.normal.xyz / 2.f + 0.5f, 1.f);
        break;
    }
    case FragmentColourType::PRIMITIVE:
    {
        output.colour = float4((input.normal + 1.f + input.tex[0]).xyz / 3.f, 1.0f);
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
