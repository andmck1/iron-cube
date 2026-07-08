#include "core_buffertypes/buffertypes.h"
#include "shader/shared/helpers.slangh"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<MVPBuffer> mvp;

[[vk::binding(2)]]
ConstantBuffer<AnimationBuffer> animation;

[[vk::binding(3)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("vertex")]
VertexOutputType main(InstancedVertexInputType input)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    VertexOutputType output;

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
    output.colour = float4((input.normal + 1.f + input.tex[0]).xyz / 3.f, 1.0f);
    return output;
}
