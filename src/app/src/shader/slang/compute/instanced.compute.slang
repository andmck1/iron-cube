#include "shader/shared/helpers.slangh"
#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

struct IndirectDrawCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    uint firstInstance;
    uint vertexOffset;
};

[[vk::binding(1)]]
ConstantBuffer<SceneBuffer> sceneData;

[[vk::binding(5)]]
RWStructuredBuffer<IndirectDrawCommand> indirectCommands;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Pass-through: all instances at LOD 0
    indirectCommands[0].indexCount = sceneData.indexCount;
    indirectCommands[0].instanceCount = sceneData.instanceCount;
    indirectCommands[0].firstIndex = 0;
    indirectCommands[0].firstInstance = 0;
    indirectCommands[0].vertexOffset = 0;

    // No instances at other LODs
    for (uint lod = 1; lod < MAX_LOD_COUNT; lod++)
    {
        indirectCommands[lod].indexCount = 0;
        indirectCommands[lod].instanceCount = 0;
        indirectCommands[lod].firstIndex = 0;
        indirectCommands[lod].firstInstance = 0;
        indirectCommands[lod].vertexOffset = 0;
    }
}
