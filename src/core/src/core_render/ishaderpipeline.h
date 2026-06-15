#pragma once

#include "core_common/types.h"
#include "core_render/irenderer.h"

class IShaderPipeline
{
    public:
    virtual ~IShaderPipeline() = default;
    virtual bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) = 0;
    virtual bool update() = 0;
    virtual bool bind() = 0;
    virtual bool recreateGraphicsPipeline() = 0;
    virtual bool shutdown() = 0;
    virtual bool draw() = 0;
    virtual PipelineStatistics getPipelineStatistics() = 0;
    virtual bool updatePipelineStatistics() = 0;
    void setIsDebugQuery(bool isDebugQuery)
    {
        isDebugQuery_ = isDebugQuery;
    };

    protected:
    bool isDebugQuery_ = false;
};
