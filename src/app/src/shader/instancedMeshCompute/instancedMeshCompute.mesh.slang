#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(InstancedMeshCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMeshCompute::SceneBuffer> scene;

[[vk::binding(InstancedMeshCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedMeshCompute::AnimationBuffer> animation;

[[vk::binding(InstancedMeshCompute::BufferSlots::DEBUGDATA)]]
RWStructuredBuffer<InstancedMeshCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedMeshCompute::BufferSlots::MODEL_VERTICES)]]
StructuredBuffer<VertexInputType> modelVertices;

[[vk::binding(InstancedMeshCompute::BufferSlots::MESHLETS)]]
StructuredBuffer<InstancedMeshCompute::OptimisedMeshlet> meshlets;

[[vk::binding(InstancedMeshCompute::BufferSlots::VERTEX_INDICES)]]
StructuredBuffer<uint> vertexIndices;

[[vk::binding(InstancedMeshCompute::BufferSlots::TRIANGLE_INDICES)]]
StructuredBuffer<uint> triangleIndices;

[[vk::binding(InstancedMeshCompute::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedMeshCompute::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

[[vk::binding(InstancedMeshCompute::BufferSlots::LOD_INSTANCES)]]
RWStructuredBuffer<ComputedInstance> computedInstances;

[[vk_push_constant]]
InstancedMeshCompute::Constants constants;

// TODO - use the defines for the triange and vertex limits??
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint gid: SV_GroupID,
    in payload InstancedMeshCompute::Payload payload,
    out indices uint3 triangles[128],
    out vertices InstancedMeshCompute::VertexOutputType vertices[64]
)
{
    uint instanceIndex = payload.instanceIndices[gid];
    uint meshletIndex = payload.meshletIndices[gid];
    InstancedMeshCompute::OptimisedMeshlet m = meshlets[meshletIndex];
    float4x4 worldMatrix = computedInstances[instanceIndex].worldMatrix;
    uint lodLevel = computedInstances[instanceIndex].lodLevel;

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
        VertexInputType vertex = modelVertices[vertexIndex];

        float4x4 mvpMatrix =
            mul(scene.projection, mul(scene.view, worldMatrix));

        float4 positionWS = mul(worldMatrix, vertex.position);
        float3 normalWS =
            normalize(mul(float3x3(worldMatrix), vertex.normal.xyz));
        float4 positionCS = mul(mvpMatrix, vertex.position);

        vertices[gtid].positionCS = positionCS;
        vertices[gtid].positionWS = positionWS;
        vertices[gtid].normalWS = { normalWS, 0.f };
        vertices[gtid].colour = float4(1.f, 1.f, 1.f, 1.f);
        vertices[gtid].uv = vertex.tex.xy;

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
            vertices[gtid].colour = (normalWS.xyzz + 1.f) / 2.f;
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
