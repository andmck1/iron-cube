#include "core_shader/shaderservice.h"
#include <spdlog/spdlog.h>

IShader *ShaderService::registerShader(
    const std::string &name, std::unique_ptr<IShader> shader
)
{
    spdlog::debug("Registering shader: {}", name);
    IShader *rawPtr = shader.get();
    shaders_[name] = std::move(shader);
    spdlog::debug("Shader registered: {}", name);
    return rawPtr;
}

IShader *ShaderService::getShader(const std::string &name)
{
    auto it = shaders_.find(name);
    if (it != shaders_.end())
        return it->second.get();
    spdlog::error("Shader not found: {}", name);
    return nullptr;
}

std::vector<IShader *> ShaderService::getAllShaders()
{
    std::vector<IShader *> result;
    for (auto &pair : shaders_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}
