#include "core_buffertypes/buffertypes.h"
#include "shader/shared/buffertypes.h"

groupshared Payload sPayload;

[shader("task")]
[numthreads(AS_GROUP_SIZE, 1, 1)]
void main(
    uint gtid: SV_GroupThreadID, uint dtid: SV_DispatchThreadID,
    uint gid: SV_GroupID
)
{
    sPayload.meshletIndices[gtid] = dtid;
    DispatchMesh(AS_GROUP_SIZE, 1, 1, sPayload);
}
