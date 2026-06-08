#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;
RWStructuredBuffer<VertexInputType> vertexBuffer;

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out indices uint3 triangles[1], out vertices VertexOutputType vertices[3]
)
{
    SetMeshOutputCounts(3, 1);
    triangles[0] = uint3(0, 1, 2);

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
    for (int i = 0; i < 3; i++)
    {
        VertexInputType vertex = vertexBuffer[i];
        vertices[i].position = mul(mvpMatrix, vertex.position);
        vertices[i].colour = (-1.f * vertexBuffer[i].normal + 1.f) / 2.f;
        vertices[i].colour[3] = 1.f;
    }
}
