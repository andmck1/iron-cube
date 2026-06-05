#include "core_ecs/component/transform.h"

#include "glm/ext/matrix_transform.hpp"
#include <spdlog/spdlog.h>

// TODO - as I use a reference to the transformMatrix elsewhere I have to update
// on change rather than on get
void TransformComponent::updateTransformMatrix() const
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
}

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
    updateTransformMatrix();
}

void TransformComponent::setPosition(const glm::vec3 &position)
{
    transform_.position = position;
    updateTransformMatrix();
}

void TransformComponent::setRotation(const glm::vec3 &rotation)
{
    transform_.rotation = rotation;
    updateTransformMatrix();
}

void TransformComponent::setScale(const glm::vec3 &scale)
{
    transform_.scale = scale;
    updateTransformMatrix();
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
    updateTransformMatrix();
    return transformMatrix_;
}

glm::mat4 *TransformComponent::getTransformMatrixRef()
{
    updateTransformMatrix();
    return &transformMatrix_;
}
