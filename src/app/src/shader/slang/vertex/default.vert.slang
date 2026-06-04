#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;

[shader("vertex")]
VertexOutputType main(VertexInputType input)
{
    VertexOutputType output;
    output.position =
        mul(mvp.projection,
            mul(mvp.view, mul(mvp.model, float4(input.position, 1.0))));
    output.colour =
        float3(input.tex.x, input.tex.y, (input.tex.x + input.tex.y) * 0.5f);
    return output;
}
