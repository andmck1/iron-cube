#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

[[vk::binding(3)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[shader("fragment")]
float4 main(VertexOutputType inVert) : SV_Target
{
    InterlockedAdd(debugData[0].nFragmentShaderInvocations, 1);
    return inVert.colour;
}
