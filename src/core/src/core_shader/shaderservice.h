#pragma once

#include "core_shader/ishaderservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class ShaderService : public IShaderService
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
