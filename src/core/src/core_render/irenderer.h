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
    long nVertices = 0;
    long nTriangles = 0;
    long nVertexShaderInvocations = 0;
    long nFragmentShaderInvocations = 0;
    long nTaskShaderInvocations = 0;
    long nMeshShaderInvocations = 0;
    long nComputeShaderInvocations = 0;
    long nDrawCalls = 0;
    long nUnusedTaskShaderInvocations = 0;
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
