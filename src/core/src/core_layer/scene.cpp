#include "core_layer/scene.h"
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
    renderer_->frame();
    return true;
}

