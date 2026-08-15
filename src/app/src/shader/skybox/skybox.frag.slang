#include "buffertypes.h"

[[vk::binding(SkyboxBufferTypes::BufferSlots::TEXTURE)]]
SamplerCube sampler;

[shader("fragment")]
float4 main(SkyboxBufferTypes::VertexOutputType inVert) : SV_Target
{
    float3 direction = normalize(inVert.direction);

    // ------------------------------------------------------------
    // Skybox
    // ------------------------------------------------------------

    float3 colour = sampler.Sample(direction).rgb;

    // ------------------------------------------------------------
    // Sun
    // ------------------------------------------------------------

    // Direction from the scene towards the sun.
    // Change this to move the sun around the sky.
    float3 sunDirection = normalize(float3(0.4, 0.25, -0.85));

    float3 sunColour = float3(1.0, 0.82, 0.55);

    float cosAngle = dot(direction, sunDirection);

    // ------------------------------------------------------------
    // Sun disc
    // ------------------------------------------------------------

    // Real sun is approximately 0.27 degrees across.
    // This is deliberately exaggerated slightly.
    float sunRadius = radians(0.5);

    float cosRadius = cos(sunRadius);
    float cosInner = cos(sunRadius * 0.7);

    float sunDisc = smoothstep(cosRadius, cosInner, cosAngle);

    // ------------------------------------------------------------
    // Sun glow
    // ------------------------------------------------------------

    // Wide, subtle glow.
    float broadGlow = pow(max(cosAngle, 0.0), 32.0);

    // Tight glow immediately around the sun.
    float tightGlow = pow(max(cosAngle, 0.0), 256.0);

    // ------------------------------------------------------------
    // HDR emission
    // ------------------------------------------------------------

    float3 sunEmission =
        sunColour * (sunDisc * 25.0 + broadGlow * 0.15 + tightGlow * 8.0);

    colour += sunEmission;

    return float4(colour, 1.0);
}
