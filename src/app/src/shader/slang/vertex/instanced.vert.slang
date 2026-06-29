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
    output.position =
        mul(mvp.projection,
            mul(mvp.view,
                mul(mvp.model, input.position + input.instancePosition)));
    output.colour = (input.normal + 1.f + input.tex[0]) / 3.f;
    // output.colour = {1.0, 1.0, 1.0, 1.0};
    return output;
}
