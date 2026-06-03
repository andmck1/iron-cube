#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[shader("vertex")]
VertexOutputType main(VertexInputType input)
{
    VertexOutputType output;
    output.position = float4(input.position, 1.0);
    output.colour =
        float3(input.tex.x, input.tex.y, (input.tex.x + input.tex.y) * 0.5f);
    return output;
}
