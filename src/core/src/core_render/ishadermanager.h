#pragma once

#include "core_render/ishader.h"
#include <memory>
#include <vector>
#include <string>

class IShaderManager
{
    public:
    virtual ~IShaderManager() = default;

    virtual IShader *registerShader(
        const std::string &name, std::unique_ptr<IShader> shader
    ) = 0;
    virtual IShader *getShader(const std::string &name) = 0;
    virtual std::vector<IShader *> getAllShaders() = 0;
};
