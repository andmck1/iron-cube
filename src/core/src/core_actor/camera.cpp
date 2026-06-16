#include "core_actor/camera.h"
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
        const glm::mat4x4 &vp = getViewProjectionMatrix();
        glm::mat4x4 invVp = glm::inverse(vp);

        glm::vec3 csFarTl = {-1.0f, 1.0f, 1.0f};
        glm::vec3 csFarBl = {-1.0f, -1.0f, 1.0f};
        glm::vec3 csFarBr = {1.0f, -1.0f, 1.0f};
        glm::vec3 csFarTr = {1.0f, 1.0f, 1.0f};

        glm::vec4 farTl = invVp * glm::vec4(csFarTl, 1.0f);
        glm::vec4 farBl = invVp * glm::vec4(csFarBl, 1.0f);
        glm::vec4 farBr = invVp * glm::vec4(csFarBr, 1.0f);
        glm::vec4 farTr = invVp * glm::vec4(csFarTr, 1.0f);

        farTl /= farTl.w;
        farBl /= farBl.w;
        farBr /= farBr.w;
        farTr /= farTr.w;

        glm::vec3 farCenter = (glm::vec3(farTl) + glm::vec3(farBl) +
                               glm::vec3(farBr) + glm::vec3(farTr)) /
                              4.0f;
        float r = glm::distance(farCenter, glm::vec3(farTl));
        cone.angle = 2.0f * std::atan(r / farClip_);
    }

    return cone;
}

std::array<FrustumPlane, 6> CameraActor::getFrustumPlanes()
{
    const glm::mat4x4 &vp = getViewProjectionMatrix();
    std::array<FrustumPlane, 6> planes;

    // Extract frustum planes from view-projection matrix
    planes[0].normal = {
        vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2],
        vp[3][3] + vp[3][2]
    };
    planes[1].normal = {
        vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2],
        vp[3][3] - vp[3][2]
    };
    planes[2].normal = {
        vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]
    };
    planes[3].normal = {
        vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]
    };
    planes[4].normal = {
        vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1],
        vp[3][3] + vp[3][1]
    };
    planes[5].normal = {
        vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1],
        vp[3][3] - vp[3][1]
    };

    // Normalize planes
    for (auto &plane : planes)
    {
        float length = glm::length(glm::vec3(plane.normal));
        plane.normal = plane.normal / length;
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

    glm::vec3 sphereCenter = eyePosition + viewDirection * (maxDistance / 2.0f);
    return glm::vec4(sphereCenter, maxDistance / 2.0f);
}
