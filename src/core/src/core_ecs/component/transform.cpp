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
    transform_.position.x = xWorldPosition;
    transform_.position.y = yWorldPosition;
    transform_.position.z = zWorldPosition;
    updateTransformMatrix();
}

void TransformComponent::moveBy(float xOffset, float yOffset, float zOffset)
{
    transform_.position.x += xOffset;
    transform_.position.y += yOffset;
    transform_.position.z += zOffset;
    updateTransformMatrix();
}

void TransformComponent::moveRelativeToForwardVectorBy(
    float xOffset, float yOffset, float zOffset
)
{
    // FILL IN
    updateTransformMatrix();
}

void TransformComponent::rotateTo(float pitch, float yaw, float roll)
{
    transform_.rotation.x = pitch;
    transform_.rotation.y = yaw;
    transform_.rotation.z = roll;

    // CLAMP
    // UPDATE FORWARD VECTOR
    updateTransformMatrix();
}

void TransformComponent::rotateBy(
    float pitchOffset, float yawOffset, float rollOffset
)
{
    transform_.rotation.x += pitchOffset;
    transform_.rotation.y += yawOffset;
    transform_.rotation.z += rollOffset;

    // CLAMP
    // UPDATE FORWARD VECTOR
    updateTransformMatrix();
}

void TransformComponent::scaleTo(float xFactor, float yFactor, float zFactor)
{
    transform_.scale.x = xFactor;
    transform_.scale.y = yFactor;
    transform_.scale.z = zFactor;

    // CLAMP to be positive
    updateTransformMatrix();
}

void TransformComponent::scaleBy(
    float xFactorOffset, float yFactorOffset, float zFactorOffset
)
{
    transform_.scale.x += xFactorOffset;
    transform_.scale.y += yFactorOffset;
    transform_.scale.z += zFactorOffset;

    // CLAMP to be positive
    updateTransformMatrix();
}
