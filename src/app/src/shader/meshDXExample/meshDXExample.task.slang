#include "buffertypes.h"
#include "shader/helpers/visibility.slangh"
#include "shader/helpers/lod.slangh"

[[vk::push_constant]]
ConstantBuffer<MeshDXExample::Constants> constants;

[[vk::binding(MeshDXExample::BufferSlots::SCENE)]]
ConstantBuffer<MeshDXExample::SceneBuffer> scene;

[[vk::binding(MeshDXExample::BufferSlots::ANIMATION)]]
ConstantBuffer<MeshDXExample::AnimationBuffer> animation;

[[vk::binding(MeshDXExample::BufferSlots::DEBUG)]]
RWStructuredBuffer<MeshDXExample::DebugDataBuffer> debugData;

[[vk::binding(MeshDXExample::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(MeshDXExample::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

[[vk::binding(MeshDXExample::BufferSlots::MESHLETS)]]
StructuredBuffer<MeshDXExample::OptimisedMeshlet> meshlets;

groupshared MeshDXExample::Payload s_Payload;

// Working groupshared memory for accumulating counts & offsets (copied to
// payload)
groupshared uint s_InstanceCounts[MAX_LOD_COUNT];
groupshared uint s_GroupOffsets[MAX_LOD_COUNT + 1];
groupshared uint s_InstanceList[TS_GROUP_SIZE];
groupshared uint s_InstanceOffsets[MAX_LOD_COUNT + 1];

// dtid = global thread index (0 -> n dispatched threads ~ nInstance * nMeshlet)
// gtid = local thread index (0 -> TS_GROUP_SIZE (32))
// gtid = LOD level here
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

    // Zero out groupshared memory which requires it.
    s_InstanceList[gtid] = 0;

    if (gtid == 0)
    {
        s_InstanceOffsets[0] = 0;
        s_GroupOffsets[0] = 0;
    }

    if (gtid < MAX_LOD_COUNT)
    {
        s_InstanceCounts[gtid] = 0;
    }

    // Cull & compact the instances
    uint lodLevel = MAX_LOD_COUNT; // LOD level of this thread's instance
    uint lodOffset = 0;            // Offset into its LOD-level instance list

    uint instanceIndex = dtid;
    if (instanceIndex < constants.workCount)
    {
        Instance instance = instances[constants.workStart + instanceIndex];
        float4 instancePosition = { instance.position, 1.f };
        float4 instanceRotation = { instance.rotation, 1.f };
        float4 instanceScale = { instance.scale, 1.f };

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
            instancePosition, instanceRotation, instanceScale
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
            if (scene.isLODOn > 0)
                lodLevel = calculateLOD(
                    worldMatrix, scene.minMeshBounds, scene.maxMeshBounds,
                    scene.eyePosition.xyz, scene.maxLODDistance.x,
                    scene.lodExponent.x, MAX_LOD_COUNT
                );
        }
    }

    // Determine counts for each LOD and each thread's offset into its LOD
    // instance list
    [unroll]
    for (uint i = 0; i < MAX_LOD_COUNT; ++i)
    {
        bool lodMatch =
            lodLevel ==
            i; // Whether this thread's calculated LOD matches the current one.

        if (lodMatch)
        {
            lodOffset = WavePrefixCountBits(lodMatch);
        }

        s_InstanceCounts[i] = WaveActiveCountBits(lodMatch);
    }

    // Compute instance and Level-of-detail
    if (gtid < MAX_LOD_COUNT)
    {
        uint lodInstanceCount = s_InstanceCounts[gtid];

        s_InstanceOffsets[gtid + 1] = lodInstanceCount;

        uint unpackedGroupCount =
            (scene.meshletLODCounts[gtid].x - 1) * lodInstanceCount;

        uint lastMeshletIndex = scene.meshletLODOffsets[gtid].x +
                                scene.meshletLODCounts[gtid].x - 1;
        MeshDXExample::OptimisedMeshlet lastMeshlet =
            meshlets[lastMeshletIndex];

        uint packCount =
            min(N_MAX_VERTICES / lastMeshlet.vertexCount,
                N_MAX_TRIANGLES / lastMeshlet.triangleCount);
        uint packedGroupCount = divRoundUp(lodInstanceCount, packCount);

        s_GroupOffsets[gtid + 1] = unpackedGroupCount + packedGroupCount;
    }

    // Accumulate LOD & instance counts to create offset lookup tables for mesh
    // shader threadgroups
    if (gtid <= MAX_LOD_COUNT)
    {
        uint instanceCount = s_InstanceOffsets[gtid];
        s_InstanceOffsets[gtid] = instanceCount + WavePrefixSum(instanceCount);

        uint lodCount = s_GroupOffsets[gtid];
        s_GroupOffsets[gtid] = lodCount + WavePrefixSum(lodCount);
    }

    // Place this thread's instance index at its assigned slot from compaction.
    if (lodLevel != MAX_LOD_COUNT)
    {
        uint lodStart = s_InstanceOffsets[lodLevel];

        s_InstanceList[lodStart + lodOffset] = instanceIndex;
    }

    // Copy groupshared memory to payload
    s_Payload.instanceList[gtid] = s_InstanceList[gtid];

    if (gtid < MAX_LOD_COUNT)
    {
        s_Payload.instanceCounts[gtid] = s_InstanceCounts[gtid];
    }

    if (gtid <= MAX_LOD_COUNT)
    {
        s_Payload.groupOffsets[gtid] = s_GroupOffsets[gtid];
        s_Payload.instanceOffsets[gtid] = s_InstanceOffsets[gtid];
    }

    // NOTE: Be aware that the maximum threadgroup count of a single dimension
    // is 65536.
    //       This method is limited to an LOD 0 size of (65536 / AS_GROUP_SIZE)
    //       meshlets.
    DispatchMesh(s_GroupOffsets[MAX_LOD_COUNT], 1, 1, s_Payload);
}

