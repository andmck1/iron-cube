#include "buffertypes.h"

[[vk::binding(InstancedVFCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedVFCompute::BufferSlots::TEXTURE)]]
Sampler2D sampler;

[shader("fragment")]
float4 main(InstancedVFCompute::VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return sampler.Sample(inVert.uv);
    // return float4(inVert.uv, 0, 1);
}

