#include "layer/scenelayer.h"
#include "service/servicelocator.h"
#include <spdlog/spdlog.h>

bool SceneLayer::initialise()
{
    spdlog::debug("Initialising SceneLayer");
    spdlog::info("Initialising scene");
    renderer_ = ServiceLocator::instance().get<IRenderer>();
    return true;
}

bool SceneLayer::render()
{
    spdlog::debug("SceneLayer rendering");
    if (!renderer_)
        return false;
    renderer_->renderScene(&scene);
    return true;
}

void SceneLayer::shutdown()
{
    spdlog::debug("SceneLayer shutting down");
    spdlog::info("SceneLayer shut down");
}
