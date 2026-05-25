#pragma once

// Shared uniform buffer object definition.
// Included by C++ (vulkanrenderer) and Slang shaders.
// Uses raw float arrays for matrices so both sides
// can interpret the layout identically.
//
// C++ side:   memcpy to/from glm::mat4 (64 bytes each)
// Slang side: float4x4 maps to 64 bytes in std140

#ifndef SHARED_UBO_H
#define SHARED_UBO_H

#ifdef __SLANG__

struct UBO
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

#else

#include <cstdint>

struct alignas(16) UBO
{
    float model[16];
    float view[16];
    float proj[16];
};

#endif // __SLANG__

#endif // SHARED_UBO_H
