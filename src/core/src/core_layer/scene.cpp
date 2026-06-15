#include "core_layer/scene.h"
#include "core_ecs/component/mesh.h"
#include "core_ecs/ecs.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_service/servicelocator.h"
#include "core_render/irenderer.h"
#include <spdlog/spdlog.h>

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
    IShaderPipelineManager *shaderPipelineManager =
        ServiceLocator::instance().get<IShaderPipelineManager>();
    if (!shaderPipelineManager)
        return false;

    for (IShaderPipeline *shaderPipeline :
         shaderPipelineManager->getAllShaderPipelines())
    {
        if (!shaderPipeline->update())
            return false;
    }

    return true;
}

bool SceneLayer::postRender()
{    IShaderPipelineManager *shaderPipelineManager =
        ServiceLocator::instance().get<IShaderPipelineManager>();
    if (!shaderPipelineManager)
        return false;

    for (const std::unique_ptr<Entity> &sceneEntity : scene.getEntities())
    {
        MeshComponent *entityMeshComponent =
            sceneEntity->getComponent<MeshComponent>();
        if (entityMeshComponent)
        {
            IShaderPipeline *shaderPipeline =
                entityMeshComponent->getShaderPipeline();
            if (!shaderPipeline)
                return false;
            if (!shaderPipeline->postRender())
                return false;
        }
    }

    return true;
}

bool SceneLayer::end()
{
    IShaderPipelineManager *shaderPipelineManager =
        ServiceLocator::instance().get<IShaderPipelineManager>();
    if (!shaderPipelineManager)
        return false;

    for (const std::unique_ptr<Entity> &sceneEntity : scene.getEntities())
    {
        MeshComponent *entityMeshComponent =
            sceneEntity->getComponent<MeshComponent>();
        if (entityMeshComponent)
        {
            IShaderPipeline *shaderPipeline =
                entityMeshComponent->getShaderPipeline();
            if (!shaderPipeline)
                return false;
            if (!shaderPipeline->updatePipelineStatistics())
                return false;
        }
    }

    return true;
}
