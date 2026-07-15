#include "buffertypes.h"

[[vk::binding(InstancedVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVF::DebugDataBuffer> debugData;

[shader("fragment")]
float4 main(InstancedVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return inVert.colour;
}
