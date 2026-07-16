#include "buffertypes.h"

[[vk::binding(InstancedMesh::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMesh::DebugDataBuffer> debugData;

[[vk::binding(InstancedMesh::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[shader("fragment")]
float4 main(InstancedMesh::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    // return inVert.colour;
    return sampler.Sample(inVert.uv);
}
