#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(PlanetBufferTypes::BufferSlots::SCENE)]]
ConstantBuffer<PlanetBufferTypes::SceneBuffer> scene;

[[vk::binding(PlanetBufferTypes::BufferSlots::MVP)]]
ConstantBuffer<PlanetBufferTypes::MVPBuffer> mvp;

// TODO - review!!
[shader("vertex")]
PlanetBufferTypes::VertexOutputType main(VertexInputType input)
{
    PlanetBufferTypes::VertexOutputType output;

    float radius = length(mvp.model[0].xyz); // uniform scale assumption
    float3 centre = float3(mvp.model[0].w, mvp.model[1].w, mvp.model[2].w);
    float3 camRight = mvp.view[0].xyz;
    float3 camUp = mvp.view[1].xyz;

    float2 localOffset = input.position.xy;

    // pad slightly so the sphere silhouette isn't clipped by quad edges
    const float pad = 10.0;
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
