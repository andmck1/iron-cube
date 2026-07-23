#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::binding(InstancedMeshCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMeshCompute::SceneBuffer> scene;

[[vk::binding(InstancedMeshCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMeshCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedMeshCompute::IndirectCommandBuffer>
    indirectCommandData;

[[vk::binding(InstancedMeshCompute::BufferSlots::INDIRECT_DRAW_COMMANDS)]]
RWStructuredBuffer<InstancedMeshCompute::MeshTasksIndirectCommand>
    indirectDrawCommands;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeDrawCommandsInvocations, 1);
    }

    uint meshletLODWorkOffset = 0;
    for (uint lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        meshletLODWorkOffset += indirectCommandData[0].lodHistogram[lodIndex] *
                                scene.meshletLODCounts[lodIndex].x;
        // TODO - use interlocked add/exchange?
        indirectCommandData[0].meshletWorkOffsets[lodIndex] =
            meshletLODWorkOffset;
    }

    uint totalGroups =
        (meshletLODWorkOffset + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;

    uint maxGroupsPerDraw = scene.maxGroupsPerDraw.x;
    if (maxGroupsPerDraw == 0)
    {
        indirectCommandData[0].indirectDrawCommandCount = 1;
        indirectDrawCommands[0].groupCountX = totalGroups;
        indirectDrawCommands[0].groupCountY = 1;
        indirectDrawCommands[0].groupCountZ = 1;
    }
    else
    {
        uint nDrawCommands =
            (totalGroups + maxGroupsPerDraw - 1) / maxGroupsPerDraw;
        nDrawCommands = min(nDrawCommands, MAX_INDIRECT_COMMAND_COUNT);
        uint remainingGroups = totalGroups;
        for (uint drawCommandIndex = 0; drawCommandIndex < nDrawCommands;
             drawCommandIndex++)
        {
            uint groupsThisDraw = min(remainingGroups, maxGroupsPerDraw);
            indirectDrawCommands[drawCommandIndex].groupCountX = groupsThisDraw;
            indirectDrawCommands[drawCommandIndex].groupCountY = 1;
            indirectDrawCommands[drawCommandIndex].groupCountZ = 1;
            remainingGroups -= groupsThisDraw;
        }
        // TODO - use interlocked add/exchange?
        indirectCommandData[0].indirectDrawCommandCount = nDrawCommands;
    }
}
