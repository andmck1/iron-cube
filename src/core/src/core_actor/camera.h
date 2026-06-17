#pragma once

#include "core_actor/iactor.h"
#include "core_buffertypes/buffertypes.h"
#include "glm/ext/matrix_float4x4.hpp"
#include <array>

class CameraActor : public IActor
{
    public:
    explicit CameraActor(const std::string &entityName) : IActor(entityName) {};
    ~CameraActor() = default;

    const glm::mat4x4 &getViewMatrix();
    glm::mat4x4 *getViewMatrixRef();
    const glm::mat4x4 &getProjectionMatrix();
    glm::mat4x4 *getProjectionMatrixRef();
    const glm::mat4x4 &getViewProjectionMatrix();
    FrustumCone getFrustumCone(bool fitFarClip = true);
    std::array<FrustumPlane, 6> getFrustumPlanes();
    glm::vec4 getFrustumSphere();

    float getNearClip() const
    {
        return nearClip_;
    }
    float getFarClip() const
    {
        return farClip_;
    }
    float getVertFovDegrees() const
    {
        return vertFovDegrees_;
    }
    float getAspect() const
    {
        return aspect_;
    }

    protected:
    void updateViewMatrix();
    void updateProjectionMatrix();

    glm::mat4x4 viewMatrix_;
    glm::mat4x4 projectionMatrix_;
    glm::mat4x4 viewProjectionMatrix_;

    // TODO - add setters/init for these
    float vertFovDegrees_ = 45.f;
    float nearClip_ = 0.1f;
    float farClip_ = 10.0f;
    float aspect_ = 16.0f / 9.0f;
};
