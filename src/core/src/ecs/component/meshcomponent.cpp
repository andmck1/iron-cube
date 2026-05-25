#include "ecs/component/meshcomponent.h"
#include "ecs/component/transformcomponent.h"
#include <spdlog/spdlog.h>

void MeshComponent::setMesh(Mesh *m)
{
    mesh = m;
}
void MeshComponent::setMaterial(Material *mat)
{
    material = mat;
}

Mesh *MeshComponent::getMesh() const
{
    return mesh;
}
Material *MeshComponent::getMaterial() const
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

    auto transform = getOwner()->getComponent<TransformComponent>();
    if (!transform)
        return;

    material->bind();
    material->setUniform("modelMatrix", transform->getTransformMatrix());
    mesh->render();
}
