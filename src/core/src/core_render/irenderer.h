#pragma once

#include "core_common/types.h"
#include "core_platform/iplatform.h"
#include "core_scene/scene.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform *platform) = 0;
    virtual bool newFrame() = 0;
    virtual bool renderScene(Scene *scene) = 0;
    virtual void shutdown() = 0;
    virtual bool renderImGui() = 0;
    virtual bool draw(Geometry *geometry) = 0;
};
