#include "core_render/vulkan/shaderpipelinemanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

// TODO - rename this to VulkanShaderPipelineManager

std::pair<IShaderPipeline *, ShaderPipelineUpdateData *>
ShaderPipelineManager::registerShaderPipeline(
    const std::string &name, std::unique_ptr<IShaderPipeline> shaderPipeline,
    std::unique_ptr<ShaderPipelineUpdateData> shaderPipelineUpdateData
)
{
    spdlog::debug("Registering shader pipeline: {}", name);
    IShaderPipeline *shaderPipelinePtr = shaderPipeline.get();
    ShaderPipelineUpdateData *shaderPipelineUpdateDataPtr =
        shaderPipelineUpdateData.get();
    shaderPipelines_[name] = std::move(shaderPipeline);
    shaderPipelineUpdateDataEntries_[name] =
        std::move(shaderPipelineUpdateData);
    spdlog::debug("Shader pipeline registered: {}", name);
    return std::pair<IShaderPipeline *, ShaderPipelineUpdateData *>(
        shaderPipelinePtr, shaderPipelineUpdateDataPtr
    );
}

IShaderPipeline *
ShaderPipelineManager::getShaderPipeline(const std::string &name)
{
    auto it = shaderPipelines_.find(name);
    if (it != shaderPipelines_.end())
        return it->second.get();
    spdlog::error("Shader pipeline not found: {}", name);
    return nullptr;
}

std::vector<IShaderPipeline *> ShaderPipelineManager::getAllShaderPipelines()
{
    std::vector<IShaderPipeline *> result;
    for (auto &pair : shaderPipelines_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}

bool ShaderPipelineManager::checkForUpdates()
{
    if (recreatePipelines)
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );
        renderer->waitAllFramesComplete();
        for (IShaderPipeline *pipeline : getAllShaderPipelines())
        {
            pipeline->recreateGraphicsPipeline();
        }
        recreatePipelines = false;
    }
    return true;
}
