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
    float sq = sqrt(disc);
    float tNear = -b - sq;
    float tFar = -b + sq;

    // if the near root is behind the camera, use the far root instead
    // (this happens when the camera is inside the sphere)
    t = (tNear > 0.0001) ? tNear : tFar;

    if (t < 0.0001)
    {
        return false; // both roots behind camera — genuinely no visible hit
    }
    return true;
}

// TODO - review
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
    float3 localNormal = normalize(hitPos - inVert.sphereCentre);

    float2 uv;
    uv.x = 0.5 + atan2(localNormal.z, localNormal.x) / (2.0 * PI);
    uv.y = 0.5 + asin(clamp(localNormal.y, -1.0, 1.0)) / PI;

    uv.x = frac(uv.x);
    uv.y = saturate(uv.y);

    float3 albedo = sampler.Sample(uv).rgb;

    float3 lightDir = normalize(-scene.directionalLight.directionVector.xyz);
    float ndotl = max(dot(normal, lightDir), 0.0);

    output.colour = float4(albedo * ndotl, 1.0);

    float4 clipPos = mul(mvp.projection, mul(mvp.view, float4(hitPos, 1.0)));
    if (clipPos.w <= 0.0001)
    {
        discard;
    }
    output.depth = saturate(clipPos.z / clipPos.w);

    return output;
}
