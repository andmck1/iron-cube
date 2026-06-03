#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[shader("fragment")]
float4 main(VertexOutputType inVert) : SV_Target
{
    float3 colour = inVert.colour;
    return float4(colour, 1.0);
}
