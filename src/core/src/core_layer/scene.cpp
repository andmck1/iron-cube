#include "core_layer/scene.h"
#include "core_ecs/component/mesh.h"
#include "core_ecs/ecs.h"
#include "core_service/servicelocator.h"
#include "core_render/irenderer.h"
#include <spdlog/spdlog.h>

// TODO - this isn't part of core, but part of the app no?
bool SceneLayer::initialise()
{
    spdlog::debug("Initialising SceneLayer");
    spdlog::info("Initialising scene");
    return true;
}

bool SceneLayer::frame()
{
    // spdlog::debug("SceneLayer rendering");
    IRenderer *renderer_ = ServiceLocator::instance().get<IRenderer>();
    if (!renderer_)
        return false;
    for (const std::unique_ptr<Entity> &sceneEntity : scene.getEntities())
    {
        MeshComponent *entityMeshComponent =
            sceneEntity->getComponent<MeshComponent>();
        if (entityMeshComponent != nullptr)
            entityMeshComponent->render();
    }
    return true;
}

bool SceneLayer::update()
{
    IRenderer *renderer_ = ServiceLocator::instance().get<IRenderer>();
    if (!renderer_)
        return false;
    for (const std::unique_ptr<Entity> &sceneEntity : scene.getEntities())
    {
        MeshComponent *entityMeshComponent =
            sceneEntity->getComponent<MeshComponent>();
        if (entityMeshComponent != nullptr)
            entityMeshComponent->update();
    }
    return true;
}
