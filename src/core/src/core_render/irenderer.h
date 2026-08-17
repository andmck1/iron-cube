#pragma once

#include "core_platform/iplatform.h"
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

struct PipelineStatistics
{
    public:
    std::vector<std::pair<std::string, long long>> stats;
};

class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    virtual bool init(IPlatform *platform) = 0;

    virtual bool beginFrame() = 0;
    virtual bool beginRender() = 0;
    virtual bool endRender() = 0;
    virtual bool drawFrame() = 0;
    virtual bool present() = 0;
    virtual bool shutdown() = 0;
    virtual bool renderImGui() = 0;
    virtual bool isWireframeMode() const = 0;
    virtual void setWireframeMode(bool enabled) = 0;
    virtual float getFrameWaitTime() = 0;
};
