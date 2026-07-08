// Core buffer types shared between app and core layers

#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#ifdef __SLANG__
#define SEMANTIC(x) : x
#else
#include <cstdint>
#include <glm/glm.hpp>
#define SEMANTIC(x)

using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;
using uint4 = glm::uvec4;
#endif

#define PI2 6.28318530719

enum FrustumPlanes : uint
{
    NEAR = 0,
    FAR = 1,
    BOTTOM = 2,
    TOP = 3,
    LEFT = 4,
    RIGHT = 5

};

enum VisibilityFunc : uint
{
    VISIBILITY_FUNC_NONE = 0,
    VISIBILITY_FUNC_PLANES = 1,
    VISIBILITY_FUNC_SPHERE = 2,
    VISIBILITY_FUNC_CONE = 3,
    VISIBILITY_FUNC_CONE_AND_NEAR_PLANE = 4,
};

struct VertexInputType
{
    float4 position;
    float4 tex;
    float4 normal;
};

struct Instance
{
    float4 position;
    float4 rotation;
    float4 scale;
};

struct FrustumPlane
{
    float4 coefficients;
};

struct FrustumCone
{
    float3 tip;
    float height;
    float3 direction;
    float angle;
};

#endif
