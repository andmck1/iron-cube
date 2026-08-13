#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(SkyboxBufferTypes::BufferSlots::MVP)]]
ConstantBuffer<SkyboxBufferTypes::MVPBuffer> mvp;

[shader("vertex")]
SkyboxBufferTypes::VertexOutputType main(VertexInputType input)
{
    SkyboxBufferTypes::VertexOutputType output;

    // Strip translation so the skybox is always centred on the camera -
    // it should rotate with the camera but never appear to move as the
    // camera translates through the world.
    float4x4 viewNoTranslation = mvp.view;
    viewNoTranslation[0][3] = 0.f;
    viewNoTranslation[1][3] = 0.f;
    viewNoTranslation[2][3] = 0.f;

    output.position =
        mul(mvp.projection,
            mul(viewNoTranslation, mul(mvp.model, input.position)));

    // Force the skybox to the far plane so it renders behind every other
    // object regardless of the cube's actual world-space size.
    output.position.z = output.position.w;

    // Cube-local vertex position doubles as the cubemap sample direction.
    output.direction = input.position.xyz;

    return output;
}
