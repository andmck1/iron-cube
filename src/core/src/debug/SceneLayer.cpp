#include "debug/SceneLayer.h"
#include "rendering/linux/VulkanRenderer.h"
#include <spdlog/spdlog.h>

SceneLayer::SceneLayer(VulkanRenderer *renderer) : renderer_(renderer)
{
}

bool SceneLayer::initialise()
{
    spdlog::info("SceneLayer initialised");
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
