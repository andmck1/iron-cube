#include "debug/SceneLayer.h"
#include "rendering/linux/VulkanRenderer.h"
#include <spdlog/spdlog.h>

SceneLayer::SceneLayer(VulkanRenderer* renderer)
    : renderer_(renderer)
{
}

bool SceneLayer::initialize()
{
    spdlog::info("SceneLayer initialized");
    return true;
}

void SceneLayer::render()
{
    if (renderer_)
    {
        renderer_->renderScene();
    }
}

void SceneLayer::shutdown()
{
    spdlog::info("SceneLayer shut down");
}
