#pragma once

#include "core_common/types.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "core_material/material.h"
#include "core_platform/iplatform.h"
#include "core_scene/scene.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform *platform) = 0;
    virtual bool initialisePipeline(const ShaderPipelineConfig &config) = 0;
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
