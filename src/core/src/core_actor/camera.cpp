#include "core_actor/camera.h"
#include "core_buffertypes/buffertypes.h"
#include "core_ecs/component/transform.h"
#include "core_platform/iplatform.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include <cmath>

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

const glm::mat4x4 &CameraActor::getProjectionMatrix()
{
    updateProjectionMatrix();
    return projectionMatrix_;
}

glm::mat4x4 *CameraActor::getProjectionMatrixRef()
{
    updateProjectionMatrix();
    return &projectionMatrix_;
}

const glm::mat4x4 &CameraActor::getViewProjectionMatrix()
{
    updateViewMatrix();
    updateProjectionMatrix();
    viewProjectionMatrix_ = projectionMatrix_ * viewMatrix_;
    return viewProjectionMatrix_;
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

void CameraActor::updateProjectionMatrix()
{
    IPlatform *platform = ServiceLocator::instance().get<IPlatform>();
    if (platform)
    {
        aspect_ = static_cast<float>(platform->getConfig().window_width) /
                  static_cast<float>(platform->getConfig().window_height);
    }
    projectionMatrix_ = glm::perspective(
        glm::radians(vertFovDegrees_), aspect_, nearClip_, farClip_
    );
}

// Some help from chaotic bob here
FrustumCone CameraActor::getFrustumCone(bool fitFarClip)
{
    FrustumCone cone = {};
    TransformComponent *transformComponent = getComponent<TransformComponent>();
    glm::mat4x4 cameraRotation = transformComponent->getRotationMatrix();
    glm::vec3 viewDirection =
        glm::vec3(cameraRotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    cone.tip = transformComponent->getPosition();
    cone.direction = viewDirection;
    cone.height = farClip_;
    cone.angle = glm::radians(
        (aspect_ > 1.0f) ? (vertFovDegrees_ * aspect_) : vertFovDegrees_
    );

    if (fitFarClip)
    {
        cone.angle = atan(5.f / (sqrtf(2.f) * farClip_));
    }

    return cone;
}

std::array<FrustumPlane, 6> CameraActor::getFrustumPlanes()
{
    std::array<FrustumPlane, 6> planes;
    const glm::mat4x4 &viewProjectionMatrix =
        glm::transpose(getViewProjectionMatrix());

    // https://gamedev.stackexchange.com/questions/156743/finding-the-normals-of-the-planes-of-a-view-frustum
    planes[FRUSTUM_PLANE_NEAR].coefficients =
        viewProjectionMatrix[2] + viewProjectionMatrix[3];

    planes[FRUSTUM_PLANE_FAR].coefficients =
        viewProjectionMatrix[3] - viewProjectionMatrix[2];

    planes[FRUSTUM_PLANE_BOTTOM].coefficients =
        viewProjectionMatrix[1] + viewProjectionMatrix[3];

    planes[FRUSTUM_PLANE_TOP].coefficients =
        viewProjectionMatrix[3] + viewProjectionMatrix[1];

    planes[FRUSTUM_PLANE_LEFT].coefficients =
        viewProjectionMatrix[0] + viewProjectionMatrix[3];

    planes[FRUSTUM_PLANE_RIGHT].coefficients =
        viewProjectionMatrix[3] + viewProjectionMatrix[0];

    // Normalise each plane so that the distance computed in the shader is
    // meaningful
    for (auto &plane : planes)
    {
        float length = glm::length(
            glm::vec3(
                plane.coefficients.x, plane.coefficients.y, plane.coefficients.z
            )
        );
        if (length > 0.0f)
        {
            plane.coefficients /= length;
        }
    }

    return planes;
}

glm::vec4 CameraActor::getFrustumSphere()
{
    TransformComponent *transformComponent = getComponent<TransformComponent>();
    glm::vec3 eyePosition = transformComponent->getPosition();
    glm::mat4x4 cameraRotation = transformComponent->getRotationMatrix();
    glm::vec3 viewDirection =
        glm::vec3(cameraRotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    float fovRadians = glm::radians(vertFovDegrees_);
    float farHalfHeight = std::tan(fovRadians / 2.0f) * farClip_;
    float farHalfWidth = farHalfHeight * aspect_;

    float maxDistance = 0.0f;

    // Check far plane corners
    glm::vec3 farCorners[4] = {
        eyePosition + viewDirection * farClip_ +
            glm::vec3(farHalfWidth, farHalfHeight, 0.0f),
        eyePosition + viewDirection * farClip_ +
            glm::vec3(farHalfWidth, -farHalfHeight, 0.0f),
        eyePosition + viewDirection * farClip_ +
            glm::vec3(-farHalfWidth, farHalfHeight, 0.0f),
        eyePosition + viewDirection * farClip_ +
            glm::vec3(-farHalfWidth, -farHalfHeight, 0.0f),
    };

    for (const auto &corner : farCorners)
    {
        float distance = glm::distance(eyePosition, corner);
        if (distance > maxDistance)
        {
            maxDistance = distance;
        }
    }

    // Check near plane corners
    float nearHalfHeight = std::tan(fovRadians / 2.0f) * nearClip_;
    float nearHalfWidth = nearHalfHeight * aspect_;

    glm::vec3 nearCorners[4] = {
        eyePosition + viewDirection * nearClip_ +
            glm::vec3(nearHalfWidth, nearHalfHeight, 0.0f),
        eyePosition + viewDirection * nearClip_ +
            glm::vec3(nearHalfWidth, -nearHalfHeight, 0.0f),
        eyePosition + viewDirection * nearClip_ +
            glm::vec3(-nearHalfWidth, nearHalfHeight, 0.0f),
        eyePosition + viewDirection * nearClip_ +
            glm::vec3(-nearHalfWidth, -nearHalfHeight, 0.0f),
    };

    for (const auto &corner : nearCorners)
    {
        float distance = glm::distance(eyePosition, corner);
        if (distance > maxDistance)
        {
            maxDistance = distance;
        }
    }

    glm::vec3 sphereCentre = eyePosition + viewDirection * (maxDistance / 2.0f);
    return glm::vec4(sphereCentre, maxDistance / 2.0f);
}
