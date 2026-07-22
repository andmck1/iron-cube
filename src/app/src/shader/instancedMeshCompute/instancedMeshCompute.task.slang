#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::push_constant]]
ConstantBuffer<InstancedMeshCompute::Constants> constants;

[[vk::binding(InstancedMeshCompute::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMeshCompute::SceneBuffer> scene;

[[vk::binding(InstancedMeshCompute::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedMeshCompute::AnimationBuffer> animation;

[[vk::binding(InstancedMeshCompute::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedMeshCompute::DebugDataBuffer> debugData;

[[vk::binding(InstancedMeshCompute::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedMeshCompute::IndirectCommandBuffer>
    indirectCommandData;

[[vk::binding(InstancedMeshCompute::BufferSlots::LOD_INSTANCES)]]
RWStructuredBuffer<ComputedInstance> computedInstances;

[[vk::binding(InstancedMeshCompute::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

groupshared InstancedMeshCompute::Payload sPayload;

// dtid = global thread index (0 -> n dispatched threads ~ nInstance * nMeshlet)
// gtid = local thread index (0 -> TS_GROUP_SIZE (32))
[shader("task")]
[numthreads(TS_GROUP_SIZE, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint dtid: SV_DispatchThreadID,
    uint gid: SV_GroupID
)
{
    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nTaskShaderInvocations, 1);
    }

    bool visible = false;

    uint globalThreadIndex = dtid + constants.workStart;
    uint lod = 0;
    uint meshletWorkOffset = 0;
    for (lod = 0; lod < MAX_LOD_COUNT; lod++)
    {
        meshletWorkOffset = indirectCommandData[0].meshletWorkOffsets[lod];
        if (globalThreadIndex < meshletWorkOffset)
            break;
    }
    uint nLODMeshlets = scene.meshletLODCounts[lod].x;

    uint lodBaseOffset =
        (lod > 0) ? indirectCommandData[0].meshletWorkOffsets[lod - 1] : 0;
    uint lodLocalOffset = globalThreadIndex - lodBaseOffset;
    uint localLODInstanceIndex = lodLocalOffset / nLODMeshlets;
    uint meshletIndex =
        lodLocalOffset % nLODMeshlets + scene.meshletLODOffsets[lod].x;
    uint instanceIndex = lod * scene.instanceCount.x + localLODInstanceIndex;

    if (dtid < constants.workCount)
    {
        float4x4 modelMatrix = computedInstances[instanceIndex].modelMatrix;
        uint lodMeshletCount = scene.meshletLODCounts[lod].x;
        if (meshletIndex < lodMeshletCount)
        {
            meshletIndex += scene.meshletLODOffsets[lod].x;

            // Don't multiply w (which is radius here) by transform
            float4 meshletBoundingSphere =
                mul(modelMatrix,
                    float4(meshletBoundingSpheres[meshletIndex].xyz, 1.f));
            meshletBoundingSphere.w = meshletBoundingSpheres[meshletIndex].w;

            visible = isVisible(
                meshletBoundingSphere, (VisibilityFunc) scene.visibilityFunc.x,
                scene.frustum.planes, scene.frustum.sphere, scene.frustum.cone
            );
        }

        uint index = WavePrefixCountBits(visible);
        sPayload.instanceIndices[index] = instanceIndex;
        sPayload.meshletIndices[index] = meshletIndex;
    }

    // This must be called from task shader
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, sPayload);
}

