#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(PlanetBufferTypes::BufferSlots::SCENE)]]
ConstantBuffer<PlanetBufferTypes::SceneBuffer> scene;

[[vk::binding(PlanetBufferTypes::BufferSlots::MVP)]]
ConstantBuffer<PlanetBufferTypes::MVPBuffer> mvp;

[shader("vertex")]
PlanetBufferTypes::VertexOutputType main(VertexInputType input)
{
    PlanetBufferTypes::VertexOutputType output;

    float3 centre = mvp.model[3].xyz;
    float radius = length(mvp.model[0].xyz); // uniform scale assumption

    float3 camRight = float3(mvp.view[0].x, mvp.view[1].x, mvp.view[2].x);
    float3 camUp = float3(mvp.view[0].y, mvp.view[1].y, mvp.view[2].y);

    float2 localOffset = input.position.xy;

    // pad slightly so the sphere silhouette isn't clipped by quad edges
    const float pad = 1.05;
    float3 worldPos = centre + camRight * localOffset.x * radius * pad +
                      camUp * localOffset.y * radius * pad;

    output.position = mul(mvp.projection, mul(mvp.view, float4(worldPos, 1.0)));
    output.uv = input.tex.xy;
    output.colour = float4(1.f, 1.f, 1.f, 1.f);

    output.rayOrigin = scene.eyePosition.xyz;
    output.rayDir = worldPos - scene.eyePosition.xyz;
    output.sphereCentre = centre;
    output.sphereRadius = radius;

    return output;
}
