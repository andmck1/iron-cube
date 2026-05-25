#include "buffers/ubuffer.h"

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

struct VertexInput
{
    float3 position : LOCATION0;
    float3 color : LOCATION1;
};

struct VertexOutput
{
    float3 color : LOCATION0;
    float4 position : SV_POSITION;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    float4 pos = float4(input.position, 1.0);
    pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, pos)));
    output.position = pos;
    output.color = input.color;
    return output;
}
