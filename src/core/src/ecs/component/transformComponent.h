#pragma once

#include "ecs/ecs.h"
#include "helpers/data.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class TransformComponent : public Component
{
    public:
    void setTransform(const Transform &transform);
    void setPosition(const glm::vec3 &position);
    void setRotation(const glm::vec3 &rotation);
    void setScale(const glm::vec3 &scale);

    const Transform &getTransform() const;
    const glm::vec3 &getPosition() const;
    const glm::vec3 &getRotation() const;
    const glm::vec3 &getScale() const;

    glm::mat4 getTransformMatrix() const;

    private:
    Transform transform_ = Transform();

    mutable glm::mat4 transformMatrix_ = glm::mat4(1.0f);
};
