#include "shader/shared/buffertypes.h"
#include "shader/shared/helpers.slangh"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<MVPBuffer> mvp;

[[vk::binding(1)]]
ConstantBuffer<AnimationBuffer> animation;

[[vk::binding(2)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("vertex")]
VertexOutputType main(VertexInputType input)
{
    // In theory you can compile this out using constants
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    VertexOutputType output;

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

    output.position =
        mul(mvp.projection, mul(mvp.view, mul(mvp.model, modelPosition)));
    output.colour = (input.normal + 1.f + input.tex[0]) / 3.f;
    return output;
}
