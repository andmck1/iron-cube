#pragma once

#include "core_actor/iactor.h"
#include "glm/ext/matrix_float4x4.hpp"

class CameraActor : public IActor
{
    public:
    explicit CameraActor(const std::string &entityName) : IActor(entityName) {};
    ~CameraActor() = default;

    const glm::mat4x4 &getViewMatrix();
    glm::mat4x4 *getViewMatrixRef();

    protected:
    void updateViewMatrix();
    glm::mat4x4 viewMatrix_;
};
