#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include <string>
class Material
{
    public:
    void bind();
    void setUniform(std::string matrixName, glm::mat4 matrix);
};
