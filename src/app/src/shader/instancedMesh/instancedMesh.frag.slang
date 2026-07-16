#include "buffertypes.h"

[[vk::binding(InstancedMesh::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMesh::DebugDataBuffer> debugData;

[shader("fragment")]
float4 main(InstancedMesh::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return inVert.colour;
}
