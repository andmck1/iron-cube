#include "buffertypes.h"
#include "helpers/maths.slangh"

[[vk::binding(MeshDXExample::BufferSlots::SCENE)]]
ConstantBuffer<MeshDXExample::SceneBuffer> scene;

[[vk::binding(MeshDXExample::BufferSlots::ANIMATION)]]
ConstantBuffer<MeshDXExample::AnimationBuffer> animation;

[[vk::binding(MeshDXExample::BufferSlots::DEBUG)]]
RWStructuredBuffer<MeshDXExample::DebugDataBuffer> debugData;

[[vk::binding(MeshDXExample::BufferSlots::MODEL_VERTICES)]]
StructuredBuffer<VertexInputType> modelVertices;

[[vk::binding(MeshDXExample::BufferSlots::MESHLETS)]]
StructuredBuffer<MeshDXExample::OptimisedMeshlet> meshlets;

[[vk::binding(MeshDXExample::BufferSlots::VERTEX_INDICES)]]
StructuredBuffer<uint> vertexIndices;

[[vk::binding(MeshDXExample::BufferSlots::TRIANGLE_INDICES)]]
StructuredBuffer<uint> triangleIndices;

[[vk::binding(MeshDXExample::BufferSlots::INSTANCES)]]
StructuredBuffer<Instance> instances;

[[vk::binding(MeshDXExample::BufferSlots::MESHLET_BOUNDING_SPHERES)]]
StructuredBuffer<float4> meshletBoundingSpheres;

[[vk_push_constant]]
MeshDXExample::Constants constants;

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10-bit-per-index triangle from a 32-bit uint.
    return uint3(
        primitive & 0xFF, (primitive >> 8) & 0xFF, (primitive >> 16) & 0xFF
    );
}

uint3 GetPrimitive(uint lodIndex, MeshDXExample::OptimisedMeshlet m, uint index)
{
    // triangleIndices is a single flat buffer; lodIndex isn't needed for
    // indexing since offsets are already global (see meshletLODOffsets usage
    // in the task shader). Kept as a parameter in case you want per-LOD
    // buffers later.
    return UnpackPrimitive(triangleIndices[m.triangleOffset + index]);
}

uint GetVertexIndex(
    uint lodIndex, MeshDXExample::OptimisedMeshlet m, uint localIndex
)
{
    // vertexIndices is StructuredBuffer<uint> (already 32-bit per index),
    // so no byte-packing/16-bit unpack logic is needed here.
    return vertexIndices[m.vertexOffset + localIndex];
}

MeshDXExample::VertexOutputType GetVertexAttributes(
    uint lodIndex, uint meshletIndex, uint vertexIndex, uint instanceIndex
)
{
    Instance n = instances[constants.workStart + instanceIndex];
    VertexInputType v = modelVertices[vertexIndex];

    float4 vertexPosition = v.position;
    float4 instancePosition = { n.position, 1.f };
    float4 instanceRotation = { n.rotation, 1.f };
    float4 instanceScale = { n.scale, 1.f };

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

    float4x4 worldMatrix =
        calculateModelMatrix(instancePosition, instanceRotation, instanceScale);

    float4x4 mvpMatrix = mul(scene.projection, mul(scene.view, worldMatrix));
    float4 positionWS = mul(mvpMatrix, vertexPosition);

    MeshDXExample::VertexOutputType vout;
    vout.position = positionWS;
    vout.colour = float4(1.f, 1.f, 1.f, 1.f); // placeholder — see note below
    vout.uv = v.tex.xy;

    return vout;
}

// TODO - use the defines for the triange and vertex limits??
[shader("mesh")]
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint gid: SV_GroupID,
    in payload MeshDXExample::Payload payload, out indices uint3 triangles[128],
    out vertices MeshDXExample::VertexOutputType vertices[64]
)
{
    // Find the LOD to which this threadgroup is assigned.
    // Each wave does this independently to avoid groupshared memory & sync.
    uint offsetCheck = 0;
    uint laneIndex = gtid % WaveGetLaneCount();

    if (laneIndex <= MAX_LOD_COUNT)
    {
        offsetCheck =
            WaveActiveCountBits(gid >= payload.groupOffsets[laneIndex]) - 1;
    }
    uint lodIndex = WaveReadLaneFirst(offsetCheck);

    // Load our LOD meshlet offset & LOD instance count
    uint lodOffset = payload.groupOffsets[lodIndex];
    uint lodCount = payload.instanceCounts[lodIndex];

    // Calculate and load our meshlet.
    uint meshletIndex =
        scene.meshletLODOffsets[lodIndex].x + (gid - lodOffset) / lodCount;
    MeshDXExample::OptimisedMeshlet m = meshlets[meshletIndex];

    // Determine instance count - only 1 instance per threadgroup in the general
    // case
    uint instanceCount = 1;

    // Last meshlet in mesh may be be packed - multiple instances rendered from
    // a single threadgroup.
    uint relativeMeshletIndex = (gid - lodOffset) / lodCount;
    uint meshletLODCount = scene.meshletLODCounts[lodIndex].x;

    if (relativeMeshletIndex == meshletLODCount - 1)
    {
        uint unpackedGroupCount = (meshletLODCount - 1) * lodCount;
        uint packedIndex = gid - (unpackedGroupCount + lodOffset);

        uint instancesPerGroup =
            min(N_MAX_VERTICES / m.vertexCount,
                N_MAX_TRIANGLES / m.triangleCount);
        uint startInstance = packedIndex * instancesPerGroup;

        instanceCount = min(lodCount - startInstance, instancesPerGroup);
    }

    // Compute our total vertex & primitive counts
    uint totalVertCount = m.vertexCount * instanceCount;
    uint totalPrimCount = m.triangleCount * instanceCount;

    SetMeshOutputCounts(totalVertCount, totalPrimCount);

    //--------------------------------------------------------------------
    // Export Primitive & Vertex Data

    if (gtid < totalVertCount)
    {
        uint readIndex =
            gtid % m.vertexCount; // Wrap our reads for packed instancing.
        uint vertexIndex = GetVertexIndex(lodIndex, m, readIndex);

        // Determine our instance index
        uint instanceId =
            gtid /
            m.vertexCount; // Instance index into this threadgroup's instances
                           // (only non-zero for packed threadgroups.)

        uint lodInstance =
            (gid - lodOffset) % lodCount +
            instanceId; // Instance index into this LOD level's instances
        uint instanceOffset =
            payload.instanceOffsets[lodIndex] +
            lodInstance; // Instance index into the payload instance list

        uint instanceIndex =
            payload.instanceList[instanceOffset]; // The final instance index of
                                                  // this vertex.

        vertices[gtid] = GetVertexAttributes(
            lodIndex, meshletIndex, vertexIndex, instanceIndex
        );

        switch ((FragmentColourType) constants.fragmentColourType)
        {
        case FragmentColourType::LOD:
        {
            float lodColour = lodIndex + 1.0;
            vertices[gtid].colour = float4(
                lodColour / MAX_LOD_COUNT, lodColour / MAX_LOD_COUNT / 3.0,
                lodColour / MAX_LOD_COUNT / 5.0, 1.f
            );
            break;
        }
        case FragmentColourType::NORMALS:
        {
            float4 normal = modelVertices[vertexIndex].normal;
            vertices[gtid].colour = (normal + 1.f) / 2.f;
            vertices[gtid].colour.w = 1.f;
            break;
        }
        case FragmentColourType::PRIMITIVE:
        {
            vertices[gtid].colour = float4(
                float(meshletIndex & 1), float(meshletIndex & 3) / 4,
                float(meshletIndex & 7) / 8, 1.0
            );
            // VISUALISE BOUNDING SPHERES
            // float4 meshletBoundingSphere =
            //     mul(worldMatrix,
            //         float4(meshletBoundingSpheres[meshletIndex].xyz, 1.f));
            // meshletBoundingSphere.w = meshletBoundingSpheres[meshletIndex].w;
            // float4 vertexWorld =
            //     mul(worldMatrix, modelVertices[vertexIndex].position);
            // vertices[gtid].colour = vertexWorld - meshletBoundingSphere;
            // vertices[gtid].colour.w = 1.f;
            break;
        }
        default:
        {
            vertices[gtid].colour = float4(1.f, 1.f, 1.f, 1.f);
            break;
        }
        }
    }

    if (gtid < totalPrimCount)
    {
        uint readIndex =
            gtid % m.triangleCount; // Wrap our reads for packed instancing.
        uint instanceId =
            gtid /
            m.triangleCount; // Instance index within this threadgroup (only
                             // non-zero in last meshlet threadgroups.)

        // Must offset the vertex indices to this thread's instanced verts
        triangles[gtid] =
            GetPrimitive(lodIndex, m, readIndex) + (m.vertexCount * instanceId);
    }
}
