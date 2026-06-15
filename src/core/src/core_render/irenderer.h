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
    int nVertices = 0;
    int nTaskShaderInvocations = 0;
    int nMeshShaderInvocations = 0;
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
};
