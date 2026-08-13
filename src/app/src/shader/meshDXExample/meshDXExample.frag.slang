#include "buffertypes.h"
#include "helpers/lighting.slangh"

[[vk::binding(MeshDXExample::BufferSlots::SCENE)]]
ConstantBuffer<MeshDXExample::SceneBuffer> scene;

[[vk::binding(MeshDXExample::BufferSlots::DEBUG)]]
RWStructuredBuffer<MeshDXExample::DebugDataBuffer> debugData;

[[vk::binding(MeshDXExample::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk_push_constant]]
MeshDXExample::Constants constants;

[shader("fragment")]
float4 main(MeshDXExample::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);

    float4 colour = inVert.colour;

    switch ((FragmentColourType) constants.fragmentColourType)
    {
    case FragmentColourType::LIGHTING:
    {
        float4 textureColour;
        float4 lightColour = { 0.f, 0.f, 0.f, 0.f };
        float2 projectedTextureCoordinate;

        textureColour = sampler.Sample(inVert.uv);
        DirectionalLight directionalLight;
        directionalLight.isEnabled = 1;
        directionalLight.diffuseColour = { 1.f, 1.f, 1.f };
        directionalLight.specularColour = { 1.f, 1.f, 1.f };
        directionalLight.specularPower = 10.f;
        directionalLight.directionVector = { 1.f, 0.f, 0.f, 0.f };

        AmbientLight ambientLight;
        ambientLight.isEnabled = 1;
        ambientLight.colour = { 1.f, 1.f, 1.f };

        if (ambientLight.isEnabled > 0)
        {
            lightColour.xyz += ambientLight.colour;
        }

        if (directionalLight.isEnabled > 0)
        {
            lightColour = calculateDirectionalLighting(
                              directionalLight, inVert.positionWS.xyz,
                              inVert.normalWS.xyz, scene.eyePosition.xyz
            )
                              .rgbb;
            lightColour[3] = 1.f;
        }
        colour = textureColour * lightColour;
        break;
    }
    case FragmentColourType::TEXTURE:
    {
        colour = sampler.Sample(inVert.uv);
        break;
    }
    case FragmentColourType::PRIMITIVE:
    {
        break;
    }
    case FragmentColourType::NORMALS:
    {
        break;
    }
    case FragmentColourType::LOD:
    {
        break;
    }
    default:
        break;
    }
    return colour;
}
