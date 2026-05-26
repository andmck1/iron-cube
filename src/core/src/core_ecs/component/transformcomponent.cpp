#include "core_ecs/component/transformcomponent.h"

#include "glm/ext/matrix_transform.hpp"
#include <spdlog/spdlog.h>

void TransformComponent::setTransform(const Transform &transform)
{
    spdlog::debug(
        "Transform updated: pos=({}, {}, {}), rot=({}, {}, {}), scale=({}, {}, "
        "{})",
        transform.position.x, transform.position.y, transform.position.z,
        transform.rotation.x, transform.rotation.y, transform.rotation.z,
        transform.scale.x, transform.scale.y, transform.scale.z
    );
    transform_ = transform;
}

void TransformComponent::setPosition(const glm::vec3 &position)
{
    transform_.position = position;
}

void TransformComponent::setRotation(const glm::vec3 &rotation)
{
    transform_.rotation = rotation;
}

void TransformComponent::setScale(const glm::vec3 &scale)
{
    transform_.scale = scale;
}

const Transform &TransformComponent::getTransform() const
{
    return transform_;
}
const glm::vec3 &TransformComponent::getPosition() const
{
    return transform_.position;
}
const glm::vec3 &TransformComponent::getRotation() const
{
    return transform_.rotation;
}
const glm::vec3 &TransformComponent::getScale() const
{
    return transform_.scale;
}

glm::mat4 TransformComponent::getTransformMatrix() const
{
    glm::mat4 translationMatrix =
        glm::translate(glm::mat4(1.0f), transform_.position);

    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix = glm::rotate(
        rotationMatrix, transform_.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)
    );

    rotationMatrix = glm::rotate(
        rotationMatrix, transform_.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)
    );

    rotationMatrix = glm::rotate(
        rotationMatrix, transform_.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)
    );

    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), transform_.scale);

    transformMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;
    return transformMatrix_;
}
