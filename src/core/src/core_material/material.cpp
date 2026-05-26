#include "core_material/material.h"
#include "core_render/irenderer.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

Material::Material()
{
}


Material::Material(ShaderPipelineConfig config) : config_(std::move(config))
{
}

void Material::bind()
{
    spdlog::debug("Binding material");
}

void Material::setUniform(std::string matrixName, glm::mat4 matrix)
{
    spdlog::debug("Setting uniform: {}", matrixName);
    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return;

    if (matrixName == "modelMatrix")
    {
        renderer->setUniformModelMatrix(matrix);
    }
}

ShaderPipelineConfig Material::getConfig() const
{
    return config_;
}
