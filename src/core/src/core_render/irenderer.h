#pragma once

#include "core_platform/iplatform.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform *platform) = 0;
    virtual bool frame() = 0;
    virtual bool renderImGui() = 0;
};
