#include "shader/shared/helpers.slangh"
#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

// Must match vk::DrawIndexedIndirectCommand layout exactly
struct IndirectDrawCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};

[[vk::binding(1)]]
ConstantBuffer<SceneBuffer> scene;

[[vk::binding(7)]]
StructuredBuffer<Instance> instances;

[[vk::binding(8)]]
RWStructuredBuffer<IndirectDrawCommand> indirectCommands;

[[vk::binding(4)]]
RWStructuredBuffer<Instance> reorderedInstances;

[[vk::binding(5)]]
RWStructuredBuffer<uint> lodBaseOffsets;

[[vk::binding(6)]]
RWStructuredBuffer<uint> lodSlotCounters;

[[vk::push_constant]]
cbuffer cbPush
{
    uint phase;
}

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

    if (phase == 0)
    {
        // Phase 0: Atomic LOD count (indirectCommands and counters cleared on
        // CPU before dispatch)
        if (instanceIndex < scene.instanceCount)
        {
            float4x4 modelMatrix = calculateModelMatrix(
                instances[instanceIndex].position,
                instances[instanceIndex].rotation, instances[instanceIndex].scale
            );
            uint lod = calculateLOD(modelMatrix);
            InterlockedAdd(indirectCommands[lod].instanceCount, 1);
        }
    }
    else
    {
        // Phase 1: Compute base offsets, scatter, set firstInstance
        // (lodSlotCounters are cleared on CPU before dispatch to avoid
        //  cross-workgroup race conditions)
        uint offset = 0;
        for (uint lod = 0; lod < MAX_LOD_COUNT; lod++)
        {
            lodBaseOffsets[lod] = offset;
            offset += indirectCommands[lod].instanceCount;
        }

        if (instanceIndex < scene.instanceCount)
        {
            float4x4 modelMatrix = calculateModelMatrix(
                instances[instanceIndex].position,
                instances[instanceIndex].rotation, instances[instanceIndex].scale
            );
            uint lod = calculateLOD(modelMatrix);
            uint baseOffset = lodBaseOffsets[lod];
            uint previousValue;
            InterlockedAdd(lodSlotCounters[lod], 1, previousValue);
            uint slot = baseOffset + previousValue;
            reorderedInstances[slot] = instances[instanceIndex];
        }

        offset = 0;
        for (uint lod = 0; lod < MAX_LOD_COUNT; lod++)
        {
            indirectCommands[lod].firstInstance = offset;
            offset += indirectCommands[lod].instanceCount;
        }
    }
}

uint calculateLOD(float4x4 modelMatrix)
{
    float4 instanceBoundsMinWS = mul(modelMatrix, scene.minMeshBounds);
    float4 instanceBoundsMaxWS = mul(modelMatrix, scene.maxMeshBounds);
    float4 instanceCentre = (instanceBoundsMinWS + instanceBoundsMaxWS) / 2.f;
    float distanceToMesh = distance(instanceCentre.xyz, scene.eyePosition.xyz);
    float normalisedDistanceToMesh =
        clamp(distanceToMesh / scene.maxLODDistance.x, 0.f, 1.f);
    uint lod = (uint) (pow(normalisedDistanceToMesh, scene.lodExponent.x) *
                       (MAX_LOD_COUNT - 1));
    return lod;
}
