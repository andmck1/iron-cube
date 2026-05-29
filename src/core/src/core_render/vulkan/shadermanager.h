#pragma once

#include "core_render/vulkan/ishadermanager.h"
#include <memory>
#include <string>
#include <unordered_map>

class ShaderManager : public IShaderManager
{
    public:
    IShader *registerShader(
        const std::string &name, std::unique_ptr<IShader> shader 
    ) override;
    IShader *getShader(const std::string &name) override;
    std::vector<IShader *> getAllShaders() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<IShader>> shaders_;
};
