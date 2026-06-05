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

glm::mat4 TransformComponent::getRotationMatrix() const
{
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    rotationMatrix =
        glm::rotate(rotationMatrix, transform_.rotation.y, glm::vec3(0, 1, 0));
    rotationMatrix =
        glm::rotate(rotationMatrix, transform_.rotation.x, glm::vec3(1, 0, 0));
    rotationMatrix =
        glm::rotate(rotationMatrix, transform_.rotation.z, glm::vec3(0, 0, 1));
    return rotationMatrix;
}

const glm::vec3 &TransformComponent::getPosition() const
{
    return transform_.position;
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

glm::vec3 TransformComponent::getForwardVector() const
{
    glm::mat4 rot = getRotationMatrix();
    return glm::normalize(glm::vec3(-rot[2]));
}

void TransformComponent::updateTransformMatrix()
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), transform_.position);
    glm::mat4 R = getRotationMatrix();
    glm::mat4 S = glm::scale(glm::mat4(1.0f), transform_.scale);

    transformMatrix_ = T * R * S;
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
    glm::mat4 rot = getRotationMatrix();

    glm::vec3 localOffset(xOffset, yOffset, zOffset);
    glm::vec3 worldOffset = glm::vec3(rot * glm::vec4(localOffset, 0.0f));

    transform_.position += worldOffset;

    updateTransformMatrix();
}

void TransformComponent::rotateTo(float pitch, float yaw, float roll)
{
    transform_.rotation = glm::vec3(pitch, yaw, roll);
    updateTransformMatrix();
}

void TransformComponent::rotateBy(
    float pitchOffset, float yawOffset, float rollOffset
)
{
    transform_.rotation += glm::vec3(pitchOffset, yawOffset, rollOffset);
    updateTransformMatrix();
}

void TransformComponent::scaleTo(float xFactor, float yFactor, float zFactor)
{
    transform_.scale.x = xFactor;
    transform_.scale.y = yFactor;
    transform_.scale.z = zFactor;

    transform_.scale.x = glm::max(transform_.scale.x, 0.001f);
    transform_.scale.y = glm::max(transform_.scale.y, 0.001f);
    transform_.scale.z = glm::max(transform_.scale.z, 0.001f);
    updateTransformMatrix();
}

void TransformComponent::scaleBy(
    float xFactorOffset, float yFactorOffset, float zFactorOffset
)
{
    transform_.scale.x += xFactorOffset;
    transform_.scale.y += yFactorOffset;
    transform_.scale.z += zFactorOffset;

    transform_.scale.x = glm::max(transform_.scale.x, 0.001f);
    transform_.scale.y = glm::max(transform_.scale.y, 0.001f);
    transform_.scale.z = glm::max(transform_.scale.z, 0.001f);
    updateTransformMatrix();
}
