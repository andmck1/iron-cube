#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<MVPBuffer> mvp;

[[vk::binding(1)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("vertex")]
VertexOutputType main(InstancedVertexInputType input)
{
    InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);

    VertexOutputType output;

    float4x4 worldMatrix = modelMatrix(
        input.instancePosition, input.instanceRotation, input.instanceScale
    );

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, worldMatrix));
    output.position = mul(mvpMatrix, input.position);
    output.colour = (input.normal + 1.f + input.tex[0]) / 3.f;
    // output.colour = {1.0, 1.0, 1.0, 1.0};
    return output;
}

// TODO - define column/row major so is always consistent
float4x4 modelMatrix(float4 T, float4 R, float4 S)
{
    float cx = cos(R.x), sx = sin(R.x);
    float cy = cos(R.y), sy = sin(R.y);
    float cz = cos(R.z), sz = sin(R.z);

    return float4x4(
        // row 0
        S.x * (cy * cz), S.y * (cy * sz), S.z * (-sy), T.x,
        // row 1
        S.x * (sx * sy * cz - cx * sz), S.y * (sx * sy * sz + cx * cz),
        S.z * (sx * cy), T.y,
        // row 2
        S.x * (cx * sy * cz + sx * sz), S.y * (cx * sy * sz - sx * cz),
        S.z * (cx * cy), T.z,
        // row 3
        0, 0, 0, 1.0
    );
}
