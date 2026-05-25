#include "material/material.h"
#include "render/irenderer.h"
#include "service/servicelocator.h"
#include <spdlog/spdlog.h>

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
