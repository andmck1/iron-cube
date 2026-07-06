#include "core_buffertypes/buffertypes.h"
#include "shader/shared/helpers.slangh"
#include "shader/shared/buffertypes.h"

[[vk::binding(0)]]
ConstantBuffer<SceneBuffer> scene;

[[vk::push_constant]]
ConstantBuffer<TaskWorkBuffer> work;

[[vk::binding(1)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

[[vk::binding(6)]]
StructuredBuffer<Instance> instances;

[[vk::binding(7)]]
StructuredBuffer<float4> meshletBoundingSpheres;

groupshared Payload sPayload;

[shader("task")]
[numthreads(AS_GROUP_SIZE, 1, 1)]
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

    uint globalThreadIndex = dtid + work.workStart;
    uint instanceIndex = globalThreadIndex / scene.meshletCount;
    uint meshletIndex = globalThreadIndex % scene.meshletCount;
    if (dtid < work.workCount)
    {
        // TODO - I do this twice!
        float4x4 modelMatrix = calculateModelMatrix(
            instances[instanceIndex].position,
            instances[instanceIndex].rotation, instances[instanceIndex].scale
        );
        uint lod = 0;
        if (scene.isLODOn > 0)
            lod = calculateLOD(modelMatrix);

        uint lodMeshletCount = scene.meshletLODCounts[lod].x;
        if (meshletIndex < lodMeshletCount)
        {
            meshletIndex += scene.meshletLODOffsets[lod].x;

            // Don't multiply w (which is radius here) by transform
            float4 meshletBoundingSphere =
                mul(modelMatrix,
                    float4(meshletBoundingSpheres[meshletIndex].xyz, 1.f));
            meshletBoundingSphere.w = meshletBoundingSpheres[meshletIndex].w;

            switch (scene.visibilityFunc)
            {
            case (VisibilityFunc::VISIBILITY_FUNC_NONE):
            {
                visible = true;
                break;
            }
            case (VisibilityFunc::VISIBILITY_FUNC_PLANES):
            {
                visible = visibleFrustumPlanes(meshletBoundingSphere);
                break;
            }
            case (VisibilityFunc::VISIBILITY_FUNC_SPHERE):
            {
                visible = visibleFrustumSphere(meshletBoundingSphere);
                break;
            }
            case (VisibilityFunc::VISIBILITY_FUNC_CONE):
            {
                visible = visibleFrustumCone(meshletBoundingSphere);
                break;
            }
            case (VisibilityFunc::VISIBILITY_FUNC_CONE_AND_NEAR_PLANE):
            {
                visible = visibleFrustumConeAndNearPlane(meshletBoundingSphere);
                break;
            }
            }
        }
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        sPayload.instanceIndices[index] = instanceIndex;
        sPayload.meshletIndices[index] = meshletIndex;
    }
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, sPayload);
}

float pointPlaneDistance(float4 testPoint, float4 planeEquation)
{
    float planeDistance =
        dot(testPoint.xyz, planeEquation.xyz) + planeEquation.w;
    return planeDistance;
};

// chaoticbob help
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
    bool inside =
        (distance(sphere.xyz, scene.frustum.sphere.xyz) <
         (sphere.w + scene.frustum.sphere.w));
    return inside;
}

bool visibleFrustumCone(float4 sphere)
{
    // float3 coneDir = { 0.f, 0.f, 1.f };
    // float3 coneTip = { 0.f, 0.f, -3.f };
    // float coneHeight = 10.f;
    // float coneAngle = 3.141f/ 10.f;
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

    FrustumPlane frNear = scene.frustum.planes[FRUSTUM_PLANE_NEAR];

    float planeDistance = pointPlaneDistance(sphere, frNear.coefficients);
    bool i1 = (abs(planeDistance) < sphere.w); // Intersects with near plane
    bool i2 = (planeDistance > 0); // On positive half space of near plane

    return i0 && (i1 || i2);
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
