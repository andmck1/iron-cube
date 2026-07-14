#include "buffertypes.h"
#include "shader/helpers.slangh"

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

[[vk::binding(InstancedVFCompute::BufferSlots::LOD_BASE_OFFSETS)]]
RWStructuredBuffer<uint> lodBaseOffsets;

[[vk::binding(InstancedVFCompute::BufferSlots::LOD_SLOT_COUNTERS)]]
RWStructuredBuffer<uint> lodSlotCounters;

[shader("compute")]
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;

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
        float4 instanceBoundingSphere =
            calculateInstanceBoundingSphere(modelMatrix);

        bool visible = isVisible(instanceBoundingSphere);
        if (visible)
        {
            uint lod = calculateLOD(modelMatrix);
            InterlockedAdd(indirectCommands[lod].instanceCount, 1);
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

float4 calculateInstanceBoundingSphere(float4x4 modelMatrix)
{
    float4 instanceBoundsMinWS = mul(modelMatrix, scene.minMeshBounds);
    float4 instanceBoundsMaxWS = mul(modelMatrix, scene.maxMeshBounds);
    float4 centre = (instanceBoundsMinWS + instanceBoundsMaxWS) / 2.f;
    float3 extent = (instanceBoundsMaxWS - instanceBoundsMinWS).xyz / 2.f;
    float radius = length(extent);
    return float4(centre.xyz, radius);
}

bool isVisible(float4 sphere)
{
    switch (scene.visibilityFunc.x)
    {
    case (VisibilityFunc::VISIBILITY_FUNC_NONE):
        return true;
    case (VisibilityFunc::VISIBILITY_FUNC_PLANES):
        return visibleFrustumPlanes(sphere);
    case (VisibilityFunc::VISIBILITY_FUNC_SPHERE):
        return visibleFrustumSphere(sphere);
    case (VisibilityFunc::VISIBILITY_FUNC_CONE):
        return visibleFrustumCone(sphere);
    case (VisibilityFunc::VISIBILITY_FUNC_CONE_AND_NEAR_PLANE):
        return visibleFrustumConeAndNearPlane(sphere);
    }
    return true;
}

float pointPlaneDistance(float4 testPoint, float4 planeEquation)
{
    return dot(testPoint.xyz, planeEquation.xyz) + planeEquation.w;
}

bool visibleFrustumPlanes(float4 sphere)
{
    for (int i = 0; i < 6; i++)
    {
        float planeDistance =
            pointPlaneDistance(sphere, scene.frustum.planes[i].coefficients);
        if (planeDistance < -sphere.w)
            return false;
    }
    return true;
}

bool visibleFrustumSphere(float4 sphere)
{
    return (
        distance(sphere.xyz, scene.frustum.sphere.xyz) <
        (sphere.w + scene.frustum.sphere.w)
    );
}

bool visibleFrustumCone(float4 sphere)
{
    float3 coneDir = scene.frustum.cone.direction.xyz;
    float3 coneTip = scene.frustum.cone.tip.xyz;
    float coneHeight = scene.frustum.cone.height.x;
    float coneAngle = scene.frustum.cone.angle.x;
    float coneRadius = coneHeight * tan(coneAngle);

    float pointDistance = dot(sphere.xyz - coneTip, coneDir);
    if ((pointDistance > coneHeight) || (pointDistance < 0.f))
        return false;

    float coneRadiusAtPoint = (pointDistance / coneHeight) * coneRadius;
    float pointConeCentreDistance =
        length((sphere.xyz - coneTip) - (pointDistance * coneDir));
    return (pointConeCentreDistance < (coneRadiusAtPoint + sphere.w));
}

bool visibleFrustumConeAndNearPlane(float4 sphere)
{
    bool i0 = visibleFrustumCone(sphere);

    FrustumPlane frNear = scene.frustum.planes[FrustumPlanes::NEAR];

    float planeDistance = pointPlaneDistance(sphere, frNear.coefficients);
    bool i1 = (abs(planeDistance) < sphere.w);
    bool i2 = (planeDistance > 0);

    return i0 && (i1 || i2);
}
