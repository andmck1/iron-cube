#include "buffertypes.h"

[[vk::binding(SkyboxBufferTypes::BufferSlots::SCENE)]]
ConstantBuffer<SkyboxBufferTypes::SceneBuffer> scene;

[[vk::binding(SkyboxBufferTypes::BufferSlots::TEXTURE)]]
SamplerCube sampler;

// TODO - review!
[shader("fragment")]
float4 main(SkyboxBufferTypes::VertexOutputType inVert) : SV_Target
{
    float3 direction = normalize(inVert.direction);

    // skybox
    float3 colour = sampler.Sample(direction).rgb;

    // sun
    float3 sunDirection =
        normalize(-scene.directionalLight.directionVector.xyz);
    float3 sunColour = float3(1.0, 0.82, 0.55);
    float cosAngle = dot(direction, sunDirection);

    float sunRadius = radians(0.5);
    float cosRadius = cos(sunRadius);
    float cosInner = cos(sunRadius * 0.7);
    float sunDisc = smoothstep(cosRadius, cosInner, cosAngle);

    float broadGlow = pow(max(cosAngle, 0.0), 32.0);
    float tightGlow = pow(max(cosAngle, 0.0), 256.0);

    float3 sunEmission =
        sunColour * (sunDisc * 25.0 + broadGlow * 0.15 + tightGlow * 8.0);

    colour += sunEmission;

    return float4(colour, 1.0);
}
