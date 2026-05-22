#include "material/material.h"
#include "render/IRenderer.h"
#include "service/ServiceLocator.h"

void Material::bind()
{
}

void Material::setUniform(std::string matrixName, glm::mat4 matrix)
{
    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return;

    if (matrixName == "modelMatrix")
    {
        renderer->setUniformModelMatrix(matrix);
    }
}
