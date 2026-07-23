#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::binding(InstancedMeshCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMeshCompute::SceneBuffer> scene;

[[vk::binding(InstancedMeshCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMeshCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedMeshCompute::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedMeshCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedMeshCompute::AnimationBuffer> animation;

[[vk::binding(InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedMeshCompute::IndirectCommandBuffer>
    indirectCommandData;

[[vk::binding(InstancedMeshCompute::BufferSlots::LOD_INSTANCES)]]
RWStructuredBuffer<ComputedInstance> computedInstances;

[shader("compute")]
[numthreads(TS_GROUP_SIZE, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeLODStatsInvocations, 1);
    }

    if (instanceIndex < scene.instanceCount.x)
    {
        float4 instancePosition = instances[instanceIndex].position;
        float4 instanceRotation = instances[instanceIndex].rotation;
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

        float4x4 worldMatrix = calculateModelMatrix(
            instancePosition, instanceRotation, instances[instanceIndex].scale
        );
        float4 instanceBoundingSphere = calculateInstanceBoundingSphere(
            worldMatrix, scene.minMeshBounds, scene.maxMeshBounds
        );

        bool visible = isVisible(
            instanceBoundingSphere, (VisibilityFunc) scene.visibilityFunc.x,
            scene.frustum.planes, scene.frustum.sphere, scene.frustum.cone
        );
        if (visible)
        {
            uint lod = calculateLOD(
                worldMatrix, scene.minMeshBounds, scene.maxMeshBounds,
                scene.eyePosition.xyz, scene.maxLODDistance.x,
                scene.lodExponent.x, MAX_LOD_COUNT
            );
            uint localLODIndex;
            InterlockedAdd(
                indirectCommandData[0].lodHistogram[lod], 1, localLODIndex
            );
            ComputedInstance computedInstance;
            computedInstance.worldMatrix = worldMatrix;
            computedInstance.lodLevel.x = lod;
            computedInstances[lod * scene.instanceCount.x + localLODIndex] =
                computedInstance;
        }
    }
}
