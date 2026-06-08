#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<MVPBuffer> mvp;

[shader("vertex")]
VertexOutputType main(VertexInputType input)
{
    VertexOutputType output;
    output.position =
        mul(mvp.projection,
            mul(mvp.view, mul(mvp.model, input.position)));
    output.colour = (input.normal + 1.f) / 2.f;
    return output;
}
