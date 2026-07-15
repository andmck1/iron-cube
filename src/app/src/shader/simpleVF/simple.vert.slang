#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(SimpleVF::BufferSlots::MVP)]]
ConstantBuffer<SimpleVF::MVPBuffer> mvp;

[[vk::binding(SimpleVF::BufferSlots::ANIMATION)]]
ConstantBuffer<SimpleVF::AnimationBuffer> animation;

[[vk::binding(SimpleVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<SimpleVF::DebugDataBuffer> debugData;

[shader("vertex")]
SimpleVF::VertexOutputType main(VertexInputType input)
{
    // In theory you can compile this out using constants
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    SimpleVF::VertexOutputType output;

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
