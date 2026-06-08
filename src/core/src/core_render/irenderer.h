#pragma once

#include "core_platform/iplatform.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform *platform) = 0;

    virtual bool beginFrame() = 0;
    virtual bool drawFrame() = 0;
    virtual bool present() = 0;
    virtual bool shutdown() = 0;
    virtual bool renderImGui() = 0;
};
