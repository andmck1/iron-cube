#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;

[[vk::binding(1)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("vertex")]
VertexOutputType main(VertexInputType input)
{
    InterlockedAdd(debugData[0].nVertexShaderInvocations, 1);

    VertexOutputType output;
    output.position =
        mul(mvp.projection,
            mul(mvp.view, mul(mvp.model, input.position)));
    output.colour = (input.normal + 1.f + input.tex[0]) / 3.f;
    return output;
}
