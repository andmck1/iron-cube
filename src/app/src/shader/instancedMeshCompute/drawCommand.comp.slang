#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::binding(InstancedMeshCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMeshCompute::SceneBuffer> scene;

[[vk::binding(InstancedMeshCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMeshCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedMeshCompute::IndirectDrawCommandBuffer>
    indirectCommandData;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeShaderInvocations, 1);
    }

    uint meshletLODWorkOffset = 0;
    for (uint lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        meshletLODWorkOffset += indirectCommandData[0].lodHistogram[lodIndex] *
                                scene.meshletLODCounts[lodIndex].x;
        indirectCommandData[0].meshletWorkOffsets[lodIndex] =
            meshletLODWorkOffset;
    }
    // TODO - this can exceed the device limit
    indirectCommandData[0].drawCommand.groupCountX =
        (meshletLODWorkOffset + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;
    indirectCommandData[0].drawCommand.groupCountY = 1;
    indirectCommandData[0].drawCommand.groupCountZ = 1;
}

