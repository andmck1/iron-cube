#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

ConstantBuffer<SceneBuffer> scene;

[[vk::binding(6)]]
RWStructuredBuffer<DebugDataBuffer> debugData;

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
        visible = true;
        sPayload.instanceIndices[gtid] = instanceIndex;
        sPayload.meshletIndices[gtid] = meshletIndex;
    }

    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, sPayload);

}
