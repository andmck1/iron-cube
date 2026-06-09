#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;
StructuredBuffer<VertexInputType> Vertices;
StructuredBuffer<OptimisedMeshlet> Meshlets;
StructuredBuffer<uint> VertexIndices;
StructuredBuffer<uint> TriangleIndices;

// TODO - use the defines for the triange and vertex limits??
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint gid: SV_GroupID,
    out indices uint3 triangles[128], out vertices VertexOutputType vertices[64]
)
{
    OptimisedMeshlet m = Meshlets[gid];
    SetMeshOutputCounts(m.VertexCount, m.TriangleCount);

    if (gtid < m.TriangleCount)
    {
        //
        // meshopt stores the triangle offset in bytes since it stores the
        // triangle indices as 3 consecutive bytes.
        //
        // Since we repacked those 3 bytes to a 32-bit uint, our offset is now
        // aligned to 4 and we can easily grab it as a uint without any
        // additional offset math.
        //
        uint packed = TriangleIndices[m.TriangleOffset + gtid];
        uint vIdx0 = (packed >> 0) & 0xFF;
        uint vIdx1 = (packed >> 8) & 0xFF;
        uint vIdx2 = (packed >> 16) & 0xFF;
        triangles[gtid] = uint3(vIdx0, vIdx1, vIdx2);
    }

    if (gtid < m.VertexCount)
    {
        uint vertexIndex = m.VertexOffset + gtid;
        vertexIndex = VertexIndices[vertexIndex];

        float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
        vertices[gtid].position =
            mul(mvpMatrix, Vertices[vertexIndex].position);

        float4 colour =
            float4(float(gid & 1), float(gid & 3) / 4, float(gid & 7) / 8, 1.f);
        vertices[gtid].colour = colour;
    }
}
