#pragma once

#include "core_render/ishaderpipelinemanager.h"
#include <memory>
#include <string>
#include <unordered_map>

class ShaderPipelineManager : public IShaderPipelineManager
{
    public:
    IShaderPipeline *registerShaderPipeline(
        const std::string &name, std::unique_ptr<IShaderPipeline> shaderPipeline
    ) override;
    IShaderPipeline *getShaderPipeline(const std::string &name) override;
    std::vector<IShaderPipeline *> getAllShaderPipelines() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<IShaderPipeline>>
        shaderpipelines_;
};
