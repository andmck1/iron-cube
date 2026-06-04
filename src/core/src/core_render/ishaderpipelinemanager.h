#pragma once

#include "core_common/types.h"
#include "core_render/ishaderpipeline.h"
#include <memory>
#include <vector>
#include <string>

class IShaderPipelineManager
{
    public:
    virtual ~IShaderPipelineManager() = default;

    virtual std::pair<IShaderPipeline *, ShaderPipelineUpdateData *>
    registerShaderPipeline(
        const std::string &name,
        std::unique_ptr<IShaderPipeline> shaderPipeline,
        std::unique_ptr<ShaderPipelineUpdateData> shaderPipelineUpdateData
    ) = 0;
    virtual IShaderPipeline *getShaderPipeline(const std::string &name) = 0;
    virtual std::vector<IShaderPipeline *> getAllShaderPipelines() = 0;
};
