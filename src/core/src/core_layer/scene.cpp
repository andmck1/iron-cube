#include "core_layer/scene.h"
#include "core_ecs/component/mesh.h"
#include "core_ecs/ecs.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

bool SceneLayer::initialise()
{
    spdlog::debug("Initialising SceneLayer");
    spdlog::info("Initialising scene");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return true;
}

bool SceneLayer::frame()
{
    // spdlog::debug("SceneLayer rendering");
    if (!renderer_)
        return false;
    renderer_->beginFrame();
    for (const std::unique_ptr<Entity> &sceneEntity : scene.getEntities())
    {
        MeshComponent *entityMeshComponent =
            sceneEntity->getComponent<MeshComponent>();
        if (entityMeshComponent != nullptr)
            entityMeshComponent->render();
    }
    renderer_->drawFrame();
    renderer_->present();
    return true;
}
