#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::binding(InstancedVFCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedVFCompute::SceneBuffer> scene;

[[vk::binding(InstancedVFCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedVFCompute::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedVFCompute::IndirectDrawCommand> indirectCommands;

[[vk::binding(InstancedVFCompute::BufferSlots::REORDERED_INSTANCES)]]
RWStructuredBuffer<Instance> reorderedInstances;

[[vk::binding(InstancedVFCompute::BufferSlots::LOD_STATS)]]
RWStructuredBuffer<InstancedVFCompute::LODStats> lodStats;

[shader("compute")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeShaderInvocations, 1);
    }

    // Guarantee this is available to all threads
    uint offset = 0;
    for (uint lod = 0; lod < MAX_LOD_COUNT; lod++)
    {
        lodStats[lod].offset = offset;
        indirectCommands[lod].firstInstance = offset;
        offset += indirectCommands[lod].instanceCount;
    }

    if (instanceIndex < scene.instanceCount.x)
    {
        float4x4 modelMatrix = calculateModelMatrix(
            instances[instanceIndex].position,
            instances[instanceIndex].rotation, instances[instanceIndex].scale
        );
        float4 instanceBoundingSphere = calculateInstanceBoundingSphere(
            modelMatrix, scene.minMeshBounds, scene.maxMeshBounds
        );

        bool visible = isVisible(
            instanceBoundingSphere, (VisibilityFunc) scene.visibilityFunc.x,
            scene.frustum.planes, scene.frustum.sphere, scene.frustum.cone
        );
        if (visible)
        {
            uint lod = calculateLOD(
                modelMatrix, scene.minMeshBounds, scene.maxMeshBounds,
                scene.eyePosition.xyz, scene.maxLODDistance.x,
                scene.lodExponent.x, MAX_LOD_COUNT
            );
            uint lodInstanceOffset = lodStats[lod].offset;

            // thread safe slot calculation
            uint previousLODInstanceCount;
            InterlockedAdd(lodStats[lod].count, 1, previousLODInstanceCount);
            uint slot = lodInstanceOffset + previousLODInstanceCount;
            reorderedInstances[slot] = instances[instanceIndex];
            reorderedInstances[slot].lodLevel.x = float(lod);
        }
    }
}

