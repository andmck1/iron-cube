#pragma once

#include "core_actor/iactor.h"
#include "glm/ext/matrix_float4x4.hpp"

class CameraActor : public IActor
{
    public:
    ~CameraActor() = default;

    const glm::mat4x4 &getViewMatrix() const;
    void setViewMatrix();

    protected:
    glm::mat4x4 viewMatrix_;
};
