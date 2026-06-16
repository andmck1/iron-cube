#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<SceneBuffer> scene;

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
    InterlockedAdd(debugData[0].nTaskShaderInvocations, 1);

    bool visible = false;

    uint instanceIndex = dtid / scene.meshletCount;
    uint meshletIndex = dtid % scene.meshletCount;

    if ((instanceIndex < scene.instanceCount) &&
        (meshletIndex < scene.meshletCount))
    {
        float4x4 model = instances[instanceIndex].transform;
        // Don't multiply w (which is radius here) by transform
        float4 meshletBoundingSphere =
            mul(model, float4(meshletBoundingSpheres[meshletIndex].xyz, 1.f));
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
            visible = VisibleFrustumPlanes(meshletBoundingSphere);
            break;
        }
        case (VisibilityFunc::VISIBILITY_FUNC_SPHERE):
        {
            visible = VisibleFrustumSphere(meshletBoundingSphere);
            break;
        }
        case (VisibilityFunc::VISIBILITY_FUNC_CONE):
        {
            visible = VisibleFrustumCone(meshletBoundingSphere);
            break;
        }
        case (VisibilityFunc::VISIBILITY_FUNC_CONE_AND_NEAR_PLANE):
        {
            visible = VisibleFrustumConeAndNearPlane(meshletBoundingSphere);
            break;
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

// chaoticbob help
float SignedPointPlaneDistance(float3 P, float3 planeN, float3 planeP)
{
    float d = dot(normalize(planeN), P - planeP);
    return d;
};

bool VisibleFrustumPlanes(float4 sphere)
{
    float radius = sphere.w;

    for (int i = 0; i < 6; i++)
    {
        float d = SignedPointPlaneDistance(
            sphere.xyz, scene.frustum.planes[0].normal.xyz,
            scene.frustum.planes[0].position.xyz
        );
        if (d < -radius)
            return false;
    }
    return true;
}

bool VisibleFrustumSphere(float4 sphere)
{
    // Intersection or inside with frustum sphere
    bool inside =
        (distance(sphere.xyz, scene.frustum.sphere.xyz) <
         (sphere.w + scene.frustum.sphere.w));
    return inside;
}

bool VisibleFrustumCone(float4 sphere)
{
    // TODO - clean up naming conventions
    // Cone and sphere are within intersectable range
    float3 v0 = sphere.xyz - scene.frustum.cone.tip;
    float d0 = dot(v0, scene.frustum.cone.direction);
    bool i0 = (d0 <= (scene.frustum.cone.height + sphere.w));

    float cs = cos(scene.frustum.cone.angle * 0.5);
    float sn = sin(scene.frustum.cone.angle * 0.5);
    float a = dot(v0, scene.frustum.cone.direction);
    float b = a * sn / cs;
    float c = sqrt(dot(v0, v0) - (a * a));
    float d = c - b;
    float e = d * cs;
    bool i1 = (e < sphere.w);

    return i0 && i1;
}

bool VisibleFrustumConeAndNearPlane(float4 sphere)
{
    bool i0 = VisibleFrustumCone(sphere);

    FrustumPlane frNear = scene.frustum.planes[FRUSTUM_PLANE_NEAR];
    float d0 = SignedPointPlaneDistance(
        sphere.xyz, frNear.normal.xyz, frNear.position.xyz
    );
    bool i1 = (abs(d0) < sphere.w); // Intersects with near plane
    bool i2 = (d0 > 0);             // On positive half space of near plane

    return i0 && (i1 || i2);
}
