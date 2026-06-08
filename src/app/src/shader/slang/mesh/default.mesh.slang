#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out indices uint3 triangles[1], out vertices VertexOutputType vertices[3]
)
{
    SetMeshOutputCounts(3, 1);
    triangles[0] = uint3(0, 1, 2);

    float4 positions[3] = {
        float4(0.f, -0.5f, 0.f, 1.f), float4(0.5f, 0.5f, 0.f, 1.f),
        float4(-0.5f, 0.5f, 0.f, 1.f)
    };

    float3 colours[3] = {
        float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0)
    };

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, mvp.model));
    for (int i = 0; i < 3; i++)
    {
        vertices[i].position = mul(mvpMatrix, positions[i]);
        // vertices[i].position = positions[i];
        vertices[i].colour = colours[i];
    }
}
