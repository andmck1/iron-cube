#include "shader/shared/helpers.slangh"
#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<SceneBuffer> scene;

[[vk::binding(2)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[[vk::binding(3)]]
StructuredBuffer<VertexInputType> modelVertices;

[[vk::binding(4)]]
StructuredBuffer<OptimisedMeshlet> meshlets;

[[vk::binding(5)]]
StructuredBuffer<uint> vertexIndices;

[[vk::binding(6)]]
StructuredBuffer<uint> triangleIndices;

[[vk::binding(7)]]
StructuredBuffer<Instance> instances;

// TODO - use the defines for the triange and vertex limits??
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint gid: SV_GroupID,
    in payload Payload payload, out indices uint3 triangles[128],
    out vertices VertexOutputType vertices[64]
)
{

    uint instanceIndex = payload.instanceIndices[gid];
    uint meshletIndex = payload.meshletIndices[gid];
    OptimisedMeshlet m = meshlets[meshletIndex];

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

        float4x4 modelMatrix = calculateModelMatrix(
            instances[instanceIndex].position,
            instances[instanceIndex].rotation, instances[instanceIndex].scale
        );

        float4x4 mvpMatrix =
            mul(scene.projection, mul(scene.view, modelMatrix));
        vertices[gtid].position =
            mul(mvpMatrix, modelVertices[vertexIndex].position);

        float4 colour =
            float4(float(gid & 1), float(gid & 3) / 4, float(gid & 7) / 8, 1.f);
        vertices[gtid].colour = colour;
    }
}
