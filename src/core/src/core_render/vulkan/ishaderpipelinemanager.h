#pragma once

#include "core_render/ishaderpipeline.h"
#include <memory>
#include <vector>
#include <string>

class IShaderPipelineManager
{
    public:
    virtual ~IShaderPipelineManager() = default;

    virtual IShaderPipeline *registerShaderPipeline(
        const std::string &name, std::unique_ptr<IShaderPipeline> shaderPipeline
    ) = 0;
    virtual IShaderPipeline *getShaderPipeline(const std::string &name) = 0;
    virtual std::vector<IShaderPipeline *> getAllShaderPipelines() = 0;
};
