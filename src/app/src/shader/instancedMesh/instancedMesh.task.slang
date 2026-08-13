#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::push_constant]]
ConstantBuffer<InstancedMesh::Constants> constants;

[[vk::binding(InstancedMesh::BufferSlots::SCENE)]]
ConstantBuffer<InstancedMesh::SceneBuffer> scene;

[[vk::binding(InstancedMesh::BufferSlots::ANIMATION)]]
ConstantBuffer<InstancedMesh::AnimationBuffer> animation;

[[vk::binding(InstancedMesh::BufferSlots::DEBUGDATA)]]
RWStructuredBuffer<InstancedMesh::DebugDataBuffer> debugData;

[[vk::binding(InstancedMesh::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedMesh::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

groupshared InstancedMesh::Payload sPayload;

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
    uint instanceIndex = globalThreadIndex / scene.meshletCount;
    uint meshletIndex = globalThreadIndex % scene.meshletCount;

    uint lod = 0;
    if (dtid < constants.workCount)
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
        // TODO - I do this twice!
        float4x4 modelMatrix = calculateModelMatrix(
            instancePosition, instanceRotation,
            float4(instances[instanceIndex].scale, 1.f)
        );

        if (scene.isLODOn > 0)
            lod = calculateLOD(
                modelMatrix, scene.minMeshBounds, scene.maxMeshBounds,
                scene.eyePosition.xyz, scene.maxLODDistance.x,
                scene.lodExponent.x, MAX_LOD_COUNT
            );

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
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        sPayload.instanceIndices[index] = instanceIndex;
        sPayload.meshletIndices[index] = meshletIndex;
        sPayload.lodLevel[index] = lod;
    }
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, sPayload);
}

