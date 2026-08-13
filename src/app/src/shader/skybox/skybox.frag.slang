#include "buffertypes.h"

[[vk::binding(SkyboxBufferTypes::BufferSlots::TEXTURE)]]
SamplerCube sampler;

[shader("fragment")]
float4 main(SkyboxBufferTypes::VertexOutputType inVert) : SV_Target
{
    return sampler.Sample(inVert.direction);
}
