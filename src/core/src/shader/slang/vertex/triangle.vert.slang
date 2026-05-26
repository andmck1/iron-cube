#include "../../../buffer/ubuffer.h"

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

[[vk::push_constant]]
uniform PushConstantData push;

struct VertexInput
{
    float3 position : LOCATION0;
    float3 colour : LOCATION1;
};

struct VertexOutput
{
    float3 colour : LOCATION0;
    float4 position : SV_POSITION;
};

[shader("vertex")]
VertexOutput main(VertexInput input)
{
    VertexOutput output;
    float4 pos = float4(input.position, 1.0);
    pos = mul(ubo.proj, mul(ubo.view, mul(push.model, pos)));
    output.position = pos;
    output.colour = input.colour;
    return output;
}
