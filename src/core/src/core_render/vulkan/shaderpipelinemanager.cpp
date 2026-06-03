#include "core_render/vulkan/shaderpipelinemanager.h"
#include <spdlog/spdlog.h>

// TODO - rename this to VulkanShaderPipelineManager

IShaderPipeline *ShaderPipelineManager::registerShaderPipeline(
    const std::string &name, std::unique_ptr<IShaderPipeline> shaderPipeline
)
{
    spdlog::debug("Registering shader pipeline: {}", name);
    IShaderPipeline *shaderPipelinePtr = shaderPipeline.get();
    shaderpipelines_[name] = std::move(shaderPipeline);
    spdlog::debug("Shader pipeline registered: {}", name);
    return shaderPipelinePtr;
}

IShaderPipeline *
ShaderPipelineManager::getShaderPipeline(const std::string &name)
{
    auto it = shaderpipelines_.find(name);
    if (it != shaderpipelines_.end())
        return it->second.get();
    spdlog::error("Shader pipeline not found: {}", name);
    return nullptr;
}

std::vector<IShaderPipeline *> ShaderPipelineManager::getAllShaderPipelines()
{
    std::vector<IShaderPipeline *> result;
    for (auto &pair : shaderpipelines_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}
