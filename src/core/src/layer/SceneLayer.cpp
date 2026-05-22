#include "layer/SceneLayer.h"
#include "service/ServiceLocator.h"
#include <spdlog/spdlog.h>

bool SceneLayer::initialise()
{
    spdlog::info("Initialising scene");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return true;
}

bool SceneLayer::render()
{
    if (!renderer_)
        return false;
    renderer_->renderScene(&scene);
    return true;
}

void SceneLayer::shutdown()
{
    spdlog::info("SceneLayer shut down");
}
