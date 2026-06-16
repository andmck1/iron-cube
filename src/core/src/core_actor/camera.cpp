#include "core_actor/camera.h"
#include "core_buffertypes/buffertypes.h"
#include "core_ecs/component/transform.h"
#include "core_platform/iplatform.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtc/matrix_access.hpp"
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
    // TODO - clean up chaotic bob code
    std::array<FrustumPlane, 6> planes;

    auto &viewProjectionMatrix = getViewProjectionMatrix();
    auto inverseVPMatrix = glm::inverse(viewProjectionMatrix);

    TransformComponent *transformComponent = getComponent<TransformComponent>();
    glm::mat4x4 cameraRotation = transformComponent->getRotationMatrix();
    glm::vec3 viewDirection =
        glm::vec3(cameraRotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    auto csNearTL = glm::vec3(-1, 1, -1);
    auto csNearBL = glm::vec3(-1, -1, -1);
    auto csNearBR = glm::vec3(1, -1, -1);
    auto csNearTR = glm::vec3(1, 1, -1);

    auto csFarTL = glm::vec3(-1, 1, 1);
    auto csFarBL = glm::vec3(-1, -1, 1);
    auto csFarBR = glm::vec3(1, -1, 1);
    auto csFarTR = glm::vec3(1, 1, 1);

    auto nearTL = inverseVPMatrix * glm::vec4(csNearTL, 1.0f);
    auto nearBL = inverseVPMatrix * glm::vec4(csNearBL, 1.0f);
    auto nearBR = inverseVPMatrix * glm::vec4(csNearBR, 1.0f);
    auto nearTR = inverseVPMatrix * glm::vec4(csNearTR, 1.0f);

    auto farTL = inverseVPMatrix * glm::vec4(csFarTL, 1.0f);
    auto farBL = inverseVPMatrix * glm::vec4(csFarBL, 1.0f);
    auto farBR = inverseVPMatrix * glm::vec4(csFarBR, 1.0f);
    auto farTR = inverseVPMatrix * glm::vec4(csFarTR, 1.0f);

    nearTL /= nearTL.w;
    nearBL /= nearBL.w;
    nearBR /= nearBR.w;
    nearTR /= nearTR.w;

    farTL /= farTL.w;
    farBL /= farBL.w;
    farBR /= farBR.w;
    farTR /= farTR.w;
    
    {
        auto nearH = glm::vec3(nearTL + nearBL) / 2.0f;
        auto farH = glm::vec3(farTL + farBL) / 2.0f;
        auto u = glm::normalize(farH - nearH);
        auto v = glm::normalize(glm::vec3(nearTL - nearBL));
        auto w = glm::cross(u, v);
        w = glm::normalize(w);

        planes[FRUSTUM_PLANE_LEFT].normal = glm::vec4(w, 0.f);
        planes[FRUSTUM_PLANE_LEFT].position =
            glm::vec4((nearH + farH) / 2.f, 1.f);
    }

    {
        auto nearH = glm::vec3(nearTR + nearBR) / 2.0f;
        auto farH = glm::vec3(farTR + farBR) / 2.0f;
        auto u = glm::normalize(farH - nearH);
        auto v = glm::normalize(glm::vec3(nearBL - nearTL));
        auto w = glm::cross(u, v);
        w = glm::normalize(w);

        planes[FRUSTUM_PLANE_RIGHT].normal = glm::vec4(w, 0.f);
        planes[FRUSTUM_PLANE_RIGHT].position =
            glm::vec4((nearH + farH) / 2.f, 1.f);
    }

    {
        auto nearH = glm::vec3(nearTL + nearTR) / 2.0f;
        auto farH = glm::vec3(farTL + farTR) / 2.0f;
        auto u = glm::normalize(farH - nearH);
        auto v = glm::normalize(glm::vec3(nearTR - nearTL));
        auto w = glm::cross(u, v);
        w = glm::normalize(w);

        planes[FRUSTUM_PLANE_TOP].normal = glm::vec4(w, 0.f);
        planes[FRUSTUM_PLANE_TOP].position =
            glm::vec4((nearH + farH) / 2.f, 1.f);
    }

    {
        auto nearH = glm::vec3(nearBL + nearBR) / 2.0f;
        auto farH = glm::vec3(farBL + farBR) / 2.0f;
        auto u = glm::normalize(farH - nearH);
        auto v = glm::normalize(glm::vec3(nearBL - nearBR));
        auto w = glm::cross(u, v);
        w = glm::normalize(w);

        planes[FRUSTUM_PLANE_BOTTOM].normal = glm::vec4(w, 0.f);
        planes[FRUSTUM_PLANE_BOTTOM].position =
            glm::vec4((nearH + farH) / 2.f, 1.f);
    }

    {
        planes[FRUSTUM_PLANE_NEAR].normal = glm::vec4(viewDirection, 0.f);
        planes[FRUSTUM_PLANE_NEAR].position = (nearTL + nearBR) / 2.f;
    }

    {
        planes[FRUSTUM_PLANE_FAR].normal = -glm::vec4(viewDirection, 0.f);
        planes[FRUSTUM_PLANE_FAR].position = (farTL + farBR) / 2.f;
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
