#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::binding(InstancedVFCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedVFCompute::SceneBuffer> scene;

[[vk::binding(InstancedVFCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedVFCompute::BufferSlots::INSTANCES)]]
StructuredBuffer<InstancedVFCompute::VFCInstance> instances;

[[vk::binding(InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedVFCompute::IndirectDrawCommand> indirectCommands;

[[vk::binding(InstancedVFCompute::BufferSlots::REORDERED_INSTANCES)]]
RWStructuredBuffer<InstancedVFCompute::VFCInstance> reorderedInstances;

[[vk::binding(InstancedVFCompute::BufferSlots::LOD_STATS)]]
RWStructuredBuffer<InstancedVFCompute::LODStats> lodStats;

[[vk::binding(InstancedVFCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedVFCompute::AnimationBuffer> animation;

[shader("compute")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeDrawCommandsInvocations, 1);
    }

    // Guarantee this is available to all threads
    uint offset = 0;
    [unroll]
    for (uint lod = 0; lod < MAX_LOD_COUNT; lod++)
    {
        lodStats[lod].offset = offset;
        indirectCommands[lod].firstInstance = offset;
        offset += indirectCommands[lod].instanceCount;
    }

    if (instanceIndex < scene.instanceCount.x)
    {
        float4x4 worldMatrix = instances[instanceIndex].worldMatrix;
        uint lod = instances[instanceIndex].lodLevel;
        bool visible = instances[instanceIndex].isVisible > 0;
        if (visible)
        {
            uint lodInstanceOffset = lodStats[lod].offset;

            // thread safe slot calculation
            uint previousLODInstanceCount;
            InterlockedAdd(lodStats[lod].count, 1, previousLODInstanceCount);
            uint slot = lodInstanceOffset + previousLODInstanceCount;
            reorderedInstances[slot] = instances[instanceIndex];
            reorderedInstances[slot].lodLevel = lod;
        }
    }
}

