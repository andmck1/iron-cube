#pragma once

#include "ecs/ecs.h"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/quaternion_float.hpp"

class TransformComponent : public Component
{
    public:
    void setPosition(const glm::vec3 &pos);

    void setRotation(const glm::quat &rot);

    void setScale(const glm::vec3 &s);

    const glm::vec3 &getPosition() const;
    const glm::quat &getRotation() const;
    const glm::vec3 &getScale() const;

    glm::mat4 getTransformMatrix() const;

    private:
    glm::vec3 position_ = glm::vec3(0.0f);
    glm::quat rotation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);

    mutable glm::mat4 transformMatrix_ = glm::mat4(1.0f);
};
