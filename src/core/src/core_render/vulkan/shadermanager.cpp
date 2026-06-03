#include "core_render/vulkan/shadermanager.h"
#include <spdlog/spdlog.h>

IShader *ShaderManager::registerShader(
    const std::string &name, std::unique_ptr<IShader> shader
)
{
    spdlog::debug("Registering shader : {}", name);
    IShader *shaderPtr = shader.get();
    shaders_[name] = std::move(shader);
    spdlog::debug("Shaderregistered: {}", name);
    return shaderPtr;
}

IShader *ShaderManager::getShader(const std::string &name)
{
    auto it = shaders_.find(name);
    if (it != shaders_.end())
        return it->second.get();
    spdlog::error("Shader not found: {}", name);
    return nullptr;
}

std::vector<IShader *> ShaderManager::getAllShaders()
{
    std::vector<IShader *> result;
    for (auto &pair : shaders_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}
