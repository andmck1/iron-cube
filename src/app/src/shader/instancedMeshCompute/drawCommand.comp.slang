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
        indirectCommandData[0].meshletWorkOffsets[lodIndex] =
            meshletLODWorkOffset;
    }

    // Total workgroups needed for all meshlet-instance work
    uint totalGroups =
        (meshletLODWorkOffset + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;

    // Split into multiple indirect draw commands, each capped at the device's
    // maxTaskWorkGroupTotalCount limit (passed via scene buffer)
    uint maxGroupsPerDraw = scene.maxGroupsPerDraw.x;
    if (maxGroupsPerDraw == 0)
    {
        // Fallback: single draw if limit not set
        indirectCommandData[0].indirectDrawCommandCount = 1;
        indirectDrawCommands[0].groupCountX = totalGroups;
        indirectDrawCommands[0].groupCountY = 1;
        indirectDrawCommands[0].groupCountZ = 1;
    }
    else
    {
        uint nCommands = 0;
        uint remainingGroups = totalGroups;
        while (remainingGroups > 0 && nCommands < MAX_INDIRECT_COMMAND_COUNT)
        {
            uint groupsThisDraw = min(remainingGroups, maxGroupsPerDraw);
            indirectDrawCommands[nCommands].groupCountX = groupsThisDraw;
            indirectDrawCommands[nCommands].groupCountY = 1;
            indirectDrawCommands[nCommands].groupCountZ = 1;
            remainingGroups -= groupsThisDraw;
            nCommands++;
        }
        indirectCommandData[0].indirectDrawCommandCount = nCommands;
    }
}
