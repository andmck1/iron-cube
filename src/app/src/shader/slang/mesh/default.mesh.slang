#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;
StructuredBuffer<VertexInputType> Vertices;
StructuredBuffer<OptimisedMeshlet> Meshlets;
StructuredBuffer<uint> VertexIndices;
StructuredBuffer<uint> TriangleIndices;

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
    uint meshletIndex = payload.meshletIndices[gid];
    OptimisedMeshlet m = Meshlets[meshletIndex];
    SetMeshOutputCounts(m.vertexCount, m.triangleCount);

    if (gtid < m.triangleCount)
    {
        uint packed = TriangleIndices[m.triangleOffset + gtid];
        uint vIdx0 = (packed >> 0) & 0xFF;
        uint vIdx1 = (packed >> 8) & 0xFF;
        uint vIdx2 = (packed >> 16) & 0xFF;
        triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
    }

    if (gtid < m.vertexCount)
    {
        uint vertexIndex = m.vertexOffset + gtid;
        vertexIndex = VertexIndices[vertexIndex];

        float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
        vertices[gtid].position =
            mul(mvpMatrix, Vertices[vertexIndex].position);

        float4 colour =
            float4(float(gid & 1), float(gid & 3) / 4, float(gid & 7) / 8, 1.f);
        vertices[gtid].colour = colour;
    }
}
