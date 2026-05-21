#include "ecs/component/transformComponent.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

void TransformComponent::setPosition(const glm::vec3 &pos)
{
    position_ = pos;
    transformDirty_ = true;
}

void TransformComponent::setRotation(const glm::quat &rot)
{
    rotation_ = rot;
    transformDirty_ = true;
}

void TransformComponent::setScale(const glm::vec3 &s)
{
    scale_ = s;
    transformDirty_ = true;
}

const glm::vec3 &TransformComponent::getPosition() const
{
    return position_;
}
const glm::quat &TransformComponent::getRotation() const
{
    return rotation_;
}
const glm::vec3 &TransformComponent::getScale() const
{
    return scale_;
}

glm::mat4 TransformComponent::getTransformMatrix() const
{
    if (transformDirty_)
    {
        glm::mat4 translationMatrix =
            glm::translate(glm::mat4(1.0f), position_);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation_);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale_);

        transformMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;
        transformDirty_ = false;
    }
    return transformMatrix_;
}
