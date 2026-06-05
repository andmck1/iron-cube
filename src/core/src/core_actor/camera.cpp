#include "core_actor/camera.h"
#include "core_ecs/component/transform.h"

const glm::mat4x4 &CameraActor::getViewMatrix() const
{
    return viewMatrix_;
}

void CameraActor::setViewMatrix()
{
    TransformComponent *transformComponent = getComponent<TransformComponent>();
    viewMatrix_ = glm::inverse(transformComponent->getTransformMatrix());
}
