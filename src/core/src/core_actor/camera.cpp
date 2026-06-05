#include "core_actor/camera.h"
#include "core_ecs/component/transform.h"
#include "glm/ext/matrix_transform.hpp"

const glm::mat4x4 &CameraActor::getViewMatrix()
{
    updateViewMatrix();
    return viewMatrix_;
}

glm::mat4x4 *CameraActor::getViewMatrixRef()
{
    updateViewMatrix();
    return &viewMatrix_;
}

void CameraActor::updateViewMatrix()
{
    TransformComponent *transformComponent = getComponent<TransformComponent>();

    glm::mat4x4 cameraRotation = transformComponent->getRotationMatrix();
    glm::vec3 eyePosition = transformComponent->getPosition();
    glm::vec3 forward =
        glm::vec3(cameraRotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    glm::vec3 up =
        glm::vec3(cameraRotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));

    glm::vec3 at = eyePosition + forward;

    viewMatrix_ = glm::lookAt(eyePosition, at, up);
}
