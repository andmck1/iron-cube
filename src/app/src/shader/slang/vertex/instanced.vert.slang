#include "core_buffertypes/buffertypes.h"
#include "shader/shared/helpers.slangh"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<MVPBuffer> mvp;

[[vk::binding(1)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("vertex")]
VertexOutputType main(InstancedVertexInputType input)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);
    }

    VertexOutputType output;

    float4x4 worldMatrix = calculateModelMatrix(
        input.instancePosition, input.instanceRotation, input.instanceScale
    );

    float4x4 mvpMatrix = mul(mvp.projection, mul(mvp.view, worldMatrix));
    output.position = mul(mvpMatrix, input.position);
    output.colour = (input.normal + 1.f + input.tex[0]) / 3.f;
    // output.colour = {1.0, 1.0, 1.0, 1.0};
    return output;
}
