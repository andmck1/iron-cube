#include "core_ecs/component/mesh.h"
#include "core_render/irenderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

void MeshComponent::setMesh(Mesh *mesh)
{
    mesh_ = mesh;
}
void MeshComponent::setShaderPipeline(IShaderPipeline *shaderPipeline)
{
    shaderPipeline_ = shaderPipeline;
}

Mesh *MeshComponent::getMesh() const
{
    return mesh_;
}
IShaderPipeline *MeshComponent::getShaderPipeline() const
{
    return shaderPipeline_;
}

bool MeshComponent::render()
{
    spdlog::debug(
        "Rendering MeshComponent for entity: {}", getOwner()->getName()
    );

    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return false;

    if (!shaderPipeline_->bind())
        return false;
    // if (!shaderPipeline_->update())
    //     return false;
    if (!shaderPipeline_->draw())
        return false;

    spdlog::debug(
        "Finished rendering MeshComponent for entity: {}", getOwner()->getName()
    );

    return true;
}
