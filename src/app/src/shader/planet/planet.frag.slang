#include "buffertypes.h"

[[vk::binding(PlanetBufferTypes::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk::binding(PlanetBufferTypes::BufferSlots::SCENE)]]
ConstantBuffer<PlanetBufferTypes::SceneBuffer> scene;

[[vk::binding(PlanetBufferTypes::BufferSlots::MVP)]]
ConstantBuffer<PlanetBufferTypes::MVPBuffer> mvp;

bool intersectSphere(
    float3 ro, float3 rd, float3 centre, float radius, out float t
)
{
    float3 oc = ro - centre;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - c;
    if (disc < 0.0)
    {
        t = 0.0;
        return false;
    }
    t = -b - sqrt(disc);
    return true;
}

[shader("fragment")]
PlanetBufferTypes::FragmentOutputType
main(PlanetBufferTypes::VertexOutputType inVert)
{
    PlanetBufferTypes::FragmentOutputType output;

    float3 rd = normalize(inVert.rayDir);
    float3 ro = inVert.rayOrigin;

    float t;
    if (!intersectSphere(ro, rd, inVert.sphereCentre, inVert.sphereRadius, t))
    {
        discard;
    }

    float3 hitPos = ro + rd * t;
    float3 normal = normalize(hitPos - inVert.sphereCentre);

    // equirectangular UV from surface normal (overrides the quad's own UV)
    float2 uv;
    uv.x = 0.5 + atan2(normal.z, normal.x) / (2.0 * 3.14159265);
    uv.y = 0.5 - asin(normal.y) / 3.14159265;

    float3 albedo = sampler.Sample(uv).rgb;

    float3 lightDir = normalize(scene.directionalLight.directionVector.xyz);
    float ndotl = max(dot(normal, lightDir), 0.0);

    output.colour = float4(albedo * ndotl, 1.0);

    float4 clipPos = mul(mvp.projection, mul(mvp.view, float4(hitPos, 1.0)));
    output.depth = saturate(clipPos.z / clipPos.w);

    return output;
}
