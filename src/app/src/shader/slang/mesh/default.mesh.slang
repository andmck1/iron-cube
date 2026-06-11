#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<SceneBuffer> scene;
StructuredBuffer<VertexInputType> modelVertices;
StructuredBuffer<OptimisedMeshlet> meshlets;
StructuredBuffer<uint> vertexIndices;
StructuredBuffer<uint> triangleIndices;
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

        float4x4 mvpMatrix =
            mul(scene.projection,
                mul(scene.view, instances[instanceIndex].transform));
        vertices[gtid].position =
            mul(mvpMatrix, modelVertices[vertexIndex].position);

        float4 colour =
            float4(float(gid & 1), float(gid & 3) / 4, float(gid & 7) / 8, 1.f);
        vertices[gtid].colour = colour;
    }
}
