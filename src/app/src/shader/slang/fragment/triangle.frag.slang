struct FragmentInput
{
    float3 colour : LOCATION0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_TARGET
{
    return float4(input.colour, 1.0);
}
