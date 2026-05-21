#pragma once

#include "scene/Scene.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(GLFWwindow *window) = 0;
    virtual void renderScene(Scene *scene) = 0;
    virtual void swapBuffers() = 0;
    virtual void shutdown() = 0;
    virtual bool initImGui() = 0;
    virtual void newFrame() = 0;
    virtual void renderImGui() = 0;
    virtual void shutdownImGui() = 0;
};
