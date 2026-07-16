#include "buffertypes.h"

[[vk::binding(InstancedVF::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVF::DebugDataBuffer> debugData;

[[vk::binding(InstancedVF::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[shader("fragment")]
float4 main(InstancedVF::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return sampler.Sample(inVert.uv);
    // return float4(inVert.uv, 0, 1);
}

