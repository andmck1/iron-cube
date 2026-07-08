#include "buffertypes.h"

[[vk::binding(SimpleVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<SimpleVF::DebugDataBuffer> debugData;

[shader("fragment")]
float4 main(SimpleVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return inVert.colour;
}
