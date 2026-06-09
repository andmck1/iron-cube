#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;
RWStructuredBuffer<VertexInputType> vertexBuffer;

[[vk::push_constant]]
cbuffer ModelData
{
    uint16_t nVertices;
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out indices uint3 triangles[1], out vertices VertexOutputType vertices[3]
)
{
    SetMeshOutputCounts(3, 1);
    triangles[0] = uint3(0, 1, 2);

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));

    // triangle fan
    for (int i = 1; i < nVertices; i++)
    {
        VertexInputType rootVertex = vertexBuffer[0];
        VertexInputType vertex = vertexBuffer[i];
        vertices[i].position = mul(mvpMatrix, vertex.position);
        vertices[i].colour = (-1.f * vertexBuffer[i].normal + 1.f) / 2.f;
        vertices[i].colour[3] = 1.f;
    }

    // for (int i = 0; i < nVertices; i++)
    // {
    //     VertexInputType vertex = vertexBuffer[i];
    //     vertices[i].position = mul(mvpMatrix, vertex.position);
    //     vertices[i].colour = (-1.f * vertexBuffer[i].normal + 1.f) / 2.f;
    //     vertices[i].colour[3] = 1.f;
    // }
}
