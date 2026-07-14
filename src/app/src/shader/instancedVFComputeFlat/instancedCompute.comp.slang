#include "buffertypes.h"
#include "shader/helpers.slangh"

[[vk::binding(InstancedVFComputeFlat::BufferSlots::SCENE)]]
ConstantBuffer<InstancedVFComputeFlat::SceneBuffer> scene;

[[vk::binding(InstancedVFComputeFlat::BufferSlots::DEBUG)]]
RWStructuredBuffer<InstancedVFComputeFlat::DebugDataBuffer> debugData;

[[vk::binding(InstancedVFComputeFlat::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(InstancedVFComputeFlat::BufferSlots::INDIRECT_COMMANDS)]]
RWStructuredBuffer<InstancedVFComputeFlat::IndirectDrawCommand>
    indirectCommands;

[[vk::binding(InstancedVFComputeFlat::BufferSlots::INDIRECT_DRAW_STATS)]]
RWStructuredBuffer<InstancedVFComputeFlat::IndirectDrawStats> indirectDrawStats;

[[vk::binding(InstancedVFComputeFlat::BufferSlots::LOD)]]
StructuredBuffer<InstancedVFComputeFlat::LOD> lods;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint instanceIndex = dispatchThreadID.x;
    uint temp;

    if (debugData[0].isDebugQueryOn > 0)
    {
        InterlockedAdd(debugData[0].nComputeShaderInvocations, 1);
    }

    if (instanceIndex == 0)
    {
        InterlockedExchange(indirectDrawStats[0].drawCount, 0, temp);
        for (uint lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
        {
            InterlockedExchange(
                indirectDrawStats[0].lodCount[lodIndex].x, 0, temp
            );
        }
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
            uint lodLevel = calculateLOD(modelMatrix);
            indirectCommands[instanceIndex].instanceCount = 1;
            indirectCommands[instanceIndex].firstIndex =
                lods[lodLevel].firstIndex;
            indirectCommands[instanceIndex].indexCount =
                lods[lodLevel].indexCount;
            InterlockedAdd(indirectDrawStats[0].lodCount[lodLevel].x, 1, temp);
        }
    }
    else
    {
        indirectCommands[instanceIndex].instanceCount = 0;
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
