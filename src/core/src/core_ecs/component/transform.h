#pragma once

#include "core_ecs/ecs.h"
#include "core_common/types.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class TransformComponent : public Component
{
    public:
    void setTransform(const Transform &transform);
    const Transform &getTransform() const;
    glm::mat4 *getTransformMatrixRef();
    glm::mat4 getRotationMatrix() const;
    const glm::vec3 &getPosition() const;
    glm::vec3 getForwardVector() const;

    void
    moveTo(float xWorldPosition, float yWorldPosition, float zWorldPosition);
    void moveBy(float xOffset, float yOffset, float zOffset);
    void
    moveRelativeToForwardVectorBy(float xOffset, float yOffset, float zOffset);

    void rotateTo(float pitch, float yaw, float roll);
    void rotateBy(float pitchOffset, float yawOffset, float rollOffset);

    void scaleTo(float xFactor, float yFactor, float zFactor);
    void scaleBy(float xFactorOffset, float yFactorOffset, float zFactorOffset);

    protected:
    void updateTransformMatrix();

    Transform transform_ = Transform();
    glm::mat4 transformMatrix_ = glm::mat4(1.0f);
};
