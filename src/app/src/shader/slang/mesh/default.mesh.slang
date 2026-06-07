#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    out indices uint3 triangles[1], out vertices VertexOutputType vertices[3]
)
{
    SetMeshOutputCounts(3, 1);
    triangles[0] = uint3(0, 1, 2);

    vertices[0].position = float4(-0.5, 0.5, 0.0, 1.0);
    vertices[0].colour = float3(1.0, 0.0, 0.0);

    vertices[1].position = float4(0.5, 0.5, 0.0, 1.0);
    vertices[1].colour = float3(0.0, 1.0, 0.0);

    vertices[2].position = float4(0.0, -0.5, 0.0, 1.0);
    vertices[2].colour = float3(0.0, 0.0, 1.0);
}
