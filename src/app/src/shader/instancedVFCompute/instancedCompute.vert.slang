#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(InstancedVFCompute::BufferSlots::MVP)]]
ConstantBuffer<InstancedVFCompute::MVPBuffer> mvp;

[[vk::binding(InstancedVFCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedVFCompute::AnimationBuffer> animation;

[[vk::binding(InstancedVFCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFCompute::DebugDataBuffer> debugData;

[[vk_push_constant]]
InstancedVFCompute::Constants constants;

[shader("vertex")]
InstancedVFCompute::VertexOutputType
main(InstancedVFCompute::InstancedVertexInputType input)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    InstancedVFCompute::VertexOutputType output;

    float4 instanceRotation = input.instanceRotation;
    float4 instancePosition = input.instancePosition;
    if (animation.isAnimationOn > 0)
    {
        float rotationAngle =
            PI2 * animation.rotationFrequency.x * animation.currentTime.x;
        instanceRotation = {
            rotateVectorWithQuaternion(
                instanceRotation.xyz,
                quaternionFromAxisAngle(float3(0.f, 1.f, 0.f), rotationAngle)
            ),
            1.f
        };

        float instanceDiscRadius = length(instancePosition.xz);
        float discAngle =
            PI2 * 1.0 * animation.currentTime.x / instanceDiscRadius;
        instancePosition =
            rotatePositionAboutOriginOnXZPlane(instancePosition, discAngle);
    }

    float4x4 worldMatrix = calculateModelMatrix(
        instancePosition, instanceRotation, input.instanceScale
    );

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, worldMatrix));
    output.position = mul(mvpMatrix, input.position);
    output.uv = input.tex.xy;

    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::LOD:
    {
        float lodColour = float(input.lodLevel.x) + 1.0;
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
