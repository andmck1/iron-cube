#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[shader("fragment")]
float4 main(VertexOutputType inVert) : SV_Target
{
    return inVert.colour;
}
