struct FragmentInput
{
    float3 color : LOCATION0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
