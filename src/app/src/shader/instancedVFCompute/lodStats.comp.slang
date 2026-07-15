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

[shader("compute")]
[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;
    uint temp;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeShaderInvocations, 1);
    }

    if (instanceIndex < scene.instanceCount.x)
    {
        float4x4 modelMatrix = calculateModelMatrix(
            instances[instanceIndex].position,
            instances[instanceIndex].rotation, instances[instanceIndex].scale
        );
        float4 instanceBoundingSphere = calculateInstanceBoundingSphere(
            modelMatrix, scene.minMeshBounds, scene.minMeshBounds
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
            InterlockedAdd(indirectCommands[lod].instanceCount, 1);
        }
    }
}
