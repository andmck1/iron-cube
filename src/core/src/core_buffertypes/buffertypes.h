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
#endif

// Matches VkVertexInputBindingDescription binding = 0
// stride = sizeof(VertexInputType) = 32 bytes
//   offset 0:  position (12 bytes)
//   offset 12: tex      (8 bytes)
//   offset 20: normal   (12 bytes)
struct VertexInputType
{
    float4 position;
    float4 tex;
    float4 normal;
};

struct VertexOutputType
{
    float4 position SEMANTIC(SV_Position);
    float4 colour;
};

struct MVPBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

#endif
