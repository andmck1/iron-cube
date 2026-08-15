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

    t = (tNear > 0.0001) ? tNear : tFar;

    if (t < 0.0001)
    {
        return false;
    }

    return true;
}

// TODO - review!
[shader("fragment")]
PlanetBufferTypes::FragmentOutputType
main(PlanetBufferTypes::VertexOutputType inVert)
{
    PlanetBufferTypes::FragmentOutputType output;

    float3 rd = normalize(inVert.rayDir);
    float3 ro = inVert.rayOrigin;

    float3 centre = inVert.sphereCentre;

    float planetRadius = inVert.sphereRadius;

    float atmosphereRadius = planetRadius * 1.10;
    float3 lightDir = normalize(-scene.directionalLight.directionVector.xyz);

    // planet intersection
    float tPlanet;
    bool hitPlanet = intersectSphere(ro, rd, centre, planetRadius, tPlanet);

    // atmosphere
    float tAtmosphere;
    bool hitAtmosphere =
        intersectSphere(ro, rd, centre, atmosphereRadius, tAtmosphere);

    if (!hitAtmosphere)
    {
        discard;
    }

    float3 atmosphereHit = ro + rd * tAtmosphere;
    float3 atmosphereNormal = normalize(atmosphereHit - centre);

    // planet
    if (hitPlanet)
    {
        float3 hitPos = ro + rd * tPlanet;

        float3 normal = normalize(hitPos - centre);
        float2 uv;
        uv.x = 0.5 + atan2(normal.z, normal.x) / (2.0 * PI);
        uv.y = 0.5 + asin(clamp(normal.y, -1.0, 1.0)) / PI;
        uv.x = frac(uv.x);
        uv.y = saturate(uv.y);
        float3 albedo = sampler.Sample(uv).rgb;

        // lighting
        float ndotl = max(dot(normal, lightDir), 0.0);
        output.colour = float4(albedo * ndotl, 1.0);

        // atmosphere on planet
        float viewDot = max(dot(normal, -rd), 0.0);
        float rim = pow(1.0 - viewDot, 1.5);
        float lightAmount = max(dot(normal, lightDir), 0.0);
        float lit = smoothstep(0.0, 0.5, lightAmount);
        float3 atmosphereColour = float3(0.3, 0.6, 1.0);
        output.colour.rgb += atmosphereColour * rim * lit * 0.8;

        // depth
        float4 clipPos =
            mul(mvp.projection, mul(mvp.view, float4(hitPos, 1.0)));
        if (clipPos.w <= 0.0001)
        {
            discard;
        }
        output.depth = saturate(clipPos.z / clipPos.w);

        return output;
    }

    // atmosphere
    float3 toCentre = inVert.sphereCentre - ro;
    float rayDistance = length(toCentre - rd * dot(toCentre, rd));
    float atmosphereRadiusRange = atmosphereRadius - planetRadius;
    float atmosphereHeight =
        saturate((rayDistance - planetRadius) / atmosphereRadiusRange);
    float atmosphereFade = 1.0 - atmosphereHeight;
    atmosphereFade = pow(atmosphereFade, 2.0);

    float viewDot = max(dot(atmosphereNormal, -rd), 0.0);
    float rim = pow(1.0 - viewDot, 1.5);

    float lightAmount = max(dot(atmosphereNormal, lightDir), 0.0);
    float lit = smoothstep(0.0, 0.5, lightAmount);

    float atmosphere = atmosphereFade * rim * lit;
    float3 atmosphereColour = float3(0.3, 0.6, 1.0);
    float atmosphereStrength = 1.5;
    output.colour = float4(atmosphereColour, atmosphere * atmosphereStrength);

    float4 atmosphereClipPos =
        mul(mvp.projection, mul(mvp.view, float4(atmosphereHit, 1.0)));
    if (atmosphereClipPos.w <= 0.0001)
    {
        discard;
    }
    output.depth = saturate(atmosphereClipPos.z / atmosphereClipPos.w);

    return output;
}
