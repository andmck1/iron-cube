/**
 * @file
 * @brief Data-oriented scene structures for vertex data storage.
 *
 * Provides Vertex and SceneData structures optimized for cache-friendly
 * access patterns. Data is stored contiguously in std::vector for
 * GPU compatibility.
 */

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cstdint>

/**
 * @brief Vertex with position and color attributes.
 *
 * Uses AoS layout: position (vec3) followed by color (vec3).
 * Total size: 24 bytes (vec3=12 + vec3=12).
 */
struct Vertex
{
    glm::vec3 pos;   ///< World space position
    glm::vec3 color; ///< RGB color values (0.0 to 1.0)
};

/**
 * @brief Triangle transform in 3D space.
 *
 * Stores position, rotation (per-axis), and scale for the triangle primitive.
 * Used by both scene layer and debug layer for transform control.
 */
struct TriangleTransform
{
    glm::vec3 position; ///< World space position
    glm::vec3 rotation; ///< Rotation angles in degrees (Euler XYZ)
    float scale;        ///< Uniform scale factor

    TriangleTransform()
        : position{0.0f, 0.0f, 0.0f}, rotation{0.0f, 0.0f, 0.0f}, scale(1.0f)
    {
    }
};

/**
 * @brief Data-oriented container for renderable geometry.
 *
 * Stores vertex data contiguously in a std::vector for cache efficiency.
 * Maintains a cached vertex count to avoid .size() calls in render hot path.
 * Tracks dirty flag for incremental buffer updates.
 */
struct SceneData
{
    std::vector<Vertex> vertices; ///< Contiguous vertex array
    uint32_t vertex_count;        ///< Cached vertex count
    bool is_dirty;                ///< True when data changed since last render

    SceneData() : vertex_count(0), is_dirty(true)
    {
    }

    /**
     * @brief Validate internal state consistency.
     * @return true if vertex_count matches vertices.size() and count > 0.
     */
    bool isValid() const
    {
        return vertex_count == static_cast<uint32_t>(vertices.size()) &&
               vertex_count > 0;
    }

    /**
     * @brief Clear all data and reset state.
     */
    void clear()
    {
        vertices.clear();
        vertex_count = 0;
        is_dirty = false;
    }
};
