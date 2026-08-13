// Core buffer types shared between app and core layers

// Make sure each variable is in a group of 16 bytes, eg.
// {
//      float4 a;
//      float3 b;
//      uint32_t _pad0;
//      float3 c;
//      float d;
// }
//
// float = 4 bytes
// float4 = 16 bytes
// uint32_t = 4 bytes
// uint4 = 16 bytes

#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#ifdef __SLANG__
#define SEMANTIC(x) : x
#else
#include <cstdint>
#include <glm/glm.hpp>
#define SEMANTIC(x)
using uint = unsigned int;
using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using float4x4 = glm::mat4;
using uint4 = glm::uvec4;
#endif

#define MAX_LOD_COUNT 5
#define PI 3.14159265359
#define PI2 6.28318530718

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
    float3 position;
    uint32_t _pad0;
    float3 rotation;
    uint32_t _pad1;
    float3 scale;
    uint32_t lodLevel;
};

struct ComputedInstance
{
    float4x4 worldMatrix;
    uint lodLevel;
    uint _pad0;
    uint _pad1;
    uint _pad2;
};

struct FrustumPlane
{
    float4 coefficients;
};

struct FrustumCone
{
    float4 tip;
    float4 height;
    float4 direction;
    float4 angle;
};

enum FragmentColourType : uint32_t
{
    LIGHTING,
    TEXTURE,
    PRIMITIVE,
    LOD,
    NORMALS,
};

struct Disc
{
    float innerRadius;
    float outerRadius;
    float height;
    float _pad0;
};

struct DirectionalLight
{
    float4 directionVector;
    float3 diffuseColour;
    uint _pad0;
    float3 specularColour;
    float specularPower;
};

struct AmbientLight
{
    float3 colour;
    uint _pad0;
};

#endif
