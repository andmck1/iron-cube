#include "core_ecs/component/meshcomponent.h"
#include "core_ecs/component/transformcomponent.h"
#include <spdlog/spdlog.h>

void MeshComponent::setMesh(Mesh *m)
{
    mesh = m;
}
void MeshComponent::setMaterial(IMaterial *mat)
{
    material = mat;
}

Mesh *MeshComponent::getMesh() const
{
    return mesh;
}
IMaterial *MeshComponent::getMaterial() const
{
    return material;
}

void MeshComponent::render()
{
    spdlog::debug(
        "Rendering MeshComponent for entity: {}", getOwner()->getName()
    );
    if (!mesh || !material)
        return;

    TransformComponent *transform =
        getOwner()->getComponent<TransformComponent>();
    if (!transform)
        return;

    material->set();

}
