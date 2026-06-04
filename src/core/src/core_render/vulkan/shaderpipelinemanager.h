#pragma once

#include "core_render/ishaderpipelinemanager.h"
#include <memory>
#include <string>
#include <unordered_map>

class ShaderPipelineManager : public IShaderPipelineManager
{
    public:
    std::pair<IShaderPipeline *, ShaderPipelineUpdateData *>
    registerShaderPipeline(
        const std::string &name,
        std::unique_ptr<IShaderPipeline> shaderPipeline,
        std::unique_ptr<ShaderPipelineUpdateData> shaderPipelineUpdateData
    ) override;
    IShaderPipeline *getShaderPipeline(const std::string &name) override;
    std::vector<IShaderPipeline *> getAllShaderPipelines() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<IShaderPipeline>>
        shaderPipelines_;
    std::unordered_map<std::string, std::unique_ptr<ShaderPipelineUpdateData>>
        shaderPipelineUpdateDataEntries_;
};
