#include "core_ecs/component/mesh.h"
#include <spdlog/spdlog.h>

void MeshComponent::setMesh(Mesh *mesh)
{
    mesh_ = mesh;
}
void MeshComponent::setShaderPipeline(IShaderPipeline *shaderPipeline)
{
    shaderPipeline = shaderPipeline;
}

Mesh *MeshComponent::getMesh() const
{
    return mesh_;
}
IShaderPipeline *MeshComponent::getShaderPipeline() const
{
    return shaderPipeline_;
}

void MeshComponent::render()
{
    spdlog::debug(
        "Rendering MeshComponent for entity: {}", getOwner()->getName()
    );
}
