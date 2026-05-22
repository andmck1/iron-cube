#pragma once

#include "common/types.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "scene/Scene.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(GLFWwindow *window) = 0;
    virtual bool renderScene(Scene *scene) = 0;
    virtual void swapBuffers() = 0;
    virtual void shutdown() = 0;
    virtual bool initImGui() = 0;
    virtual void shutdownImGui() = 0;
    virtual bool newFrame() = 0;
    virtual bool renderImGui() = 0;

    virtual bool drawGeometry(Geometry *geometry) = 0;
    virtual void setUniformModelMatrix(glm::mat4 model) = 0;
    virtual void setDeltaTime(float deltaSeconds) = 0;
};
