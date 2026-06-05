#include "core_ecs/component/transform.h"

#include "glm/ext/matrix_transform.hpp"
#include <spdlog/spdlog.h>

const Transform &TransformComponent::getTransform() const
{
    return transform_;
}

glm::mat4 *TransformComponent::getTransformMatrixRef()
{
    updateTransformMatrix();
    return &transformMatrix_;
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

void TransformComponent::updateTransformMatrix()
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

void TransformComponent::moveTo(
    float xWorldPosition, float yWorldPosition, float zWorldPosition
)
{
}

void TransformComponent::moveBy(float xOffset, float yOffset, float zOffset)
{
}
void TransformComponent::moveRelativeToForwardVectorBy(
    float xOffset, float yOffset, float zOffset
)
{
}

void TransformComponent::rotateTo(float pitch, float yaw, float roll)
{
}

void TransformComponent::rotateBy(
    float pitchOffset, float yawOffset, float rollOffset
)
{
}

void TransformComponent::scaleTo(float xFactor, float yFactor, float zFactor)
{
}

void TransformComponent::scaleBy(
    float xFactorOffset, float yFactorOffset, float zFactorOffset
)
{
}
