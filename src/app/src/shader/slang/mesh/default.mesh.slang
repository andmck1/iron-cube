#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

static const int VERTEX_LIMIT = 64;
static const int TRIANGLE_LIMIT = 126;

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
    out indices uint3 triangles[TRIANGLE_LIMIT],
    out vertices VertexOutputType vertices[VERTEX_LIMIT]
)
{
}
