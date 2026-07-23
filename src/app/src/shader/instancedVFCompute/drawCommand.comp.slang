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
    for (uint lod = 0; lod < MAX_LOD_COUNT; lod++)
    {
        lodStats[lod].offset = offset;
        indirectCommands[lod].firstInstance = offset;
        offset += indirectCommands[lod].instanceCount;
    }

    if (instanceIndex < scene.instanceCount.x)
    {

        float4 instancePosition = { instances[instanceIndex].position, 1.f };
        float4 instanceRotation = { instances[instanceIndex].rotation, 1.f };
        if (animation.isAnimationOn > 0)
        {
            float rotationAngle =
                PI2 * animation.rotationFrequency.x * animation.currentTime.x;
            float4 spin =
                quaternionFromAxisAngle(float3(0.f, 1.f, 0.f), rotationAngle);
            instanceRotation = quaternionMultiply(spin, instanceRotation);

            float instanceDiscRadius = length(instancePosition.xz);
            float discAngle =
                PI2 * 1.0 * animation.currentTime.x / instanceDiscRadius;
            instancePosition =
                rotatePositionAboutOriginOnXZPlane(instancePosition, discAngle);
        }

        float4x4 modelMatrix = calculateModelMatrix(
            instancePosition, instanceRotation,
            float4(instances[instanceIndex].scale, 1.f)
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
            reorderedInstances[slot].lodLevel = lod;
        }
    }
}

