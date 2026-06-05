#include "core_actor/camera.h"
#include "core_ecs/component/transform.h"

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
    viewMatrix_ = glm::inverse(transformComponent->getTransformMatrix());
}
