#include "buffertypes.h"

[[vk::binding(SimpleVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<SimpleVF::DebugDataBuffer> debugData;

[[vk::binding(SimpleVF::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[shader("fragment")]
float4 main(SimpleVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return sampler.Sample(inVert.uv);
    // return float4(inVert.uv, 0, 1);
}
