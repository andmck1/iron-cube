#pragma once

#include "core_platform/iplatform.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

struct PipelineStatistics
{
    public:
    enum PipelineType
    {
        TRADITIONAL,
        MESH
    };

    PipelineType pipelineType = PipelineType::TRADITIONAL;
    double nVertices = 0;
    double nTriangles = 0;
    double nVertexShaderInvocations = 0;
    double nFragmentShaderInvocations = 0;
    double nTaskShaderInvocations = 0;
    double nMeshShaderInvocations = 0;
    double nDrawCalls = 0;
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
