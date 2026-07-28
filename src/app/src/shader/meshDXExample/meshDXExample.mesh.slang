#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(MeshDXExample::BufferSlots::SCENE)]]
ConstantBuffer<MeshDXExample::SceneBuffer> scene;

[[vk::binding(MeshDXExample::BufferSlots::ANIMATION)]]
ConstantBuffer<MeshDXExample::AnimationBuffer> animation;

[[vk::binding(MeshDXExample::BufferSlots::DEBUG)]]
RWStructuredBuffer<MeshDXExample::DebugDataBuffer> debugData;

[[vk::binding(MeshDXExample::BufferSlots::MODEL_VERTICES)]]
StructuredBuffer<VertexInputType> modelVertices;

[[vk::binding(MeshDXExample::BufferSlots::MESHLETS)]]
StructuredBuffer<MeshDXExample::OptimisedMeshlet> meshlets;

[[vk::binding(MeshDXExample::BufferSlots::VERTEX_INDICES)]]
StructuredBuffer<uint> vertexIndices;

[[vk::binding(MeshDXExample::BufferSlots::TRIANGLE_INDICES)]]
StructuredBuffer<uint> triangleIndices;

[[vk::binding(MeshDXExample::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(MeshDXExample::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

[[vk_push_constant]]
MeshDXExample::Constants constants;

// TODO - use the defines for the triange and vertex limits??
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint gid: SV_GroupID,
    in payload MeshDXExample::Payload payload, out indices uint3 triangles[128],
    out vertices MeshDXExample::VertexOutputType vertices[64]
)
{
    uint instanceIndex = payload.instanceIndices[gid];
    uint meshletIndex = payload.meshletIndices[gid];
    float lodLevel = payload.lodLevel[gid];
    MeshDXExample::OptimisedMeshlet m = meshlets[meshletIndex];

    SetMeshOutputCounts(m.vertexCount, m.triangleCount);
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nMeshShaderInvocations, 1);
    }

    if (gtid == 0)
    {
        if (debugData[0].isDebugQueryOn > 0)
        {
            InterlockedAdd(debugData[0].nTriangles, m.triangleCount);
        }
    }

    if (gtid < m.triangleCount)
    {
        uint packed = triangleIndices[m.triangleOffset + gtid];
        uint vIdx0 = (packed >> 0) & 0xFF;
        uint vIdx1 = (packed >> 8) & 0xFF;
        uint vIdx2 = (packed >> 16) & 0xFF;
        triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
    }

    if (gtid < m.vertexCount)
    {
        uint vertexIndex = m.vertexOffset + gtid;
        vertexIndex = vertexIndices[vertexIndex];

        float4 instanceRotation = { instances[instanceIndex].rotation, 1.f };
        float4 instancePosition = { instances[instanceIndex].position, 1.f };
        if (animation.isAnimationOn > 0)
        {
            float rotationAngle =
                PI2 * animation.rotationFrequency.x * animation.currentTime.x;
            float4 spin =
                quaternionFromAxisAngle(float3(0.f, 1.f, 0.f), rotationAngle);
            instanceRotation = quaternionMultiply(spin, instanceRotation);

            float instanceDiscRadius = length(instancePosition.xz);
            float discAngle =
                PI2 * 1.0 * animation.currentTime.x / instanceDiscRadius;
            instancePosition =
                rotatePositionAboutOriginOnXZPlane(instancePosition, discAngle);
        }

        float4x4 worldMatrix = calculateModelMatrix(
            instancePosition, instanceRotation,
            float4(instances[instanceIndex].scale, 1.f)
        );

        float4x4 mvpMatrix =
            mul(scene.projection, mul(scene.view, worldMatrix));
        vertices[gtid].position =
            mul(mvpMatrix, modelVertices[vertexIndex].position);
        vertices[gtid].uv = modelVertices[vertexIndex].tex.xy;

        float4 normal = modelVertices[vertexIndex].normal;

        switch ((FragmentColourType) constants.fragmentColourType)
        {
        case FragmentColourType::LOD:
        {
            float lodColour = lodLevel + 1.0;
            vertices[gtid].colour = float4(
                lodColour / MAX_LOD_COUNT, lodColour / MAX_LOD_COUNT / 3.0,
                lodColour / MAX_LOD_COUNT / 5.0, 1.f
            );
            break;
        }
        case FragmentColourType::NORMALS:
        {
            vertices[gtid].colour = (normal + 1.f) / 2.f;
            vertices[gtid].colour.w = 1.f;
            break;
        }
        case FragmentColourType::PRIMITIVE:
        {
            vertices[gtid].colour = float4(
                float(meshletIndex & 1), float(meshletIndex & 3) / 4,
                float(meshletIndex & 7) / 8, 1.0
            );
            // VISUALISE BOUNDING SPHERES
            // float4 meshletBoundingSphere =
            //     mul(worldMatrix,
            //         float4(meshletBoundingSpheres[meshletIndex].xyz, 1.f));
            // meshletBoundingSphere.w = meshletBoundingSpheres[meshletIndex].w;
            // float4 vertexWorld =
            //     mul(worldMatrix, modelVertices[vertexIndex].position);
            // vertices[gtid].colour = vertexWorld - meshletBoundingSphere;
            // vertices[gtid].colour.w = 1.f;
            break;
        }
        default:
        {
            vertices[gtid].colour = float4(1.f, 1.f, 1.f, 1.f);
            break;
        }
        }
    }
}
