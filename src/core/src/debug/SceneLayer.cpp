#include "debug/SceneLayer.h"
#include "rendering/IRenderer.h"
#include "service/ServiceLocator.h"
#include <spdlog/spdlog.h>

bool SceneLayer::initialise()
{
    spdlog::info("Initialising scene");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return scene.initialise();
}

void SceneLayer::render()
{
    if (renderer_)
    {
        renderer_->renderScene(&scene);
    }
}

void SceneLayer::shutdown()
{
    spdlog::info("SceneLayer shut down");
}
