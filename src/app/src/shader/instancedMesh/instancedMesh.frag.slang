#include "buffertypes.h"
#include "helpers/lighting.slangh"

[[vk::binding(InstancedMesh::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMesh::SceneBuffer> scene;

[[vk::binding(InstancedMesh::BufferSlots::DEBUGDATA)]]
RWStructuredBuffer<InstancedMesh::DebugDataBuffer> debugData;

[[vk::binding(InstancedMesh::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[[vk_push_constant]]
InstancedMesh::Constants constants;

[shader("fragment")]
float4 main(InstancedMesh::VertexOutputType inVert) : SV_Target
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

        lightColour.xyz +=
            scene.ambientLight.colour * scene.ambientLight.intensity;

        lightColour += calculateDirectionalLighting(
                           scene.directionalLight, inVert.positionWS.xyz,
                           inVert.normalWS.xyz, scene.eyePosition.xyz
        )
                           .rgbb;
        lightColour[3] = 1.f;
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
