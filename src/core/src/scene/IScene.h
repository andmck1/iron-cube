/**
 * @file
 * @brief Scene service interface for geometry management.
 *
 * Defines the contract for scene data providers consumed via
 * the service locator. Implementations procedurally generate
 * or load vertex geometry for rendering.
 */

#pragma once

#include "scene/SceneData.h"
#include <vector>
#include <cstdint>

/**
 * @brief Scene service interface.
 *
 * Abstracts scene data generation and management.
 * Clients request scene data from the locator rather than
 * constructing scene objects directly.
 *
 * @par Thread safety
 * All methods must be called from the main thread.
 */
class IScene
{
    public:
    virtual ~IScene() = default;

    /**
     * @brief Initialise and generate scene geometry data.
     * @return true on success.
     */
    virtual bool initialise() = 0;

    /**
     * @brief Return reference to vertex data array.
     * @return Const reference to the vertex vector.
     */
    virtual const std::vector<Vertex> &getVertices() const = 0;

    /**
     * @brief Return number of vertices in scene.
     * @return Vertex count.
     */
    virtual uint32_t getVertexCount() const = 0;

    /**
     * @brief Clear all scene data and release resources.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get current triangle transform.
     * @return TriangleTransform with position, rotation, scale.
     */
    virtual TriangleTransform getTriangleTransform() const = 0;

    /**
     * @brief Set triangle transform.
     * @param transform New transform values.
     */
    virtual void setTriangleTransform(const TriangleTransform &transform) = 0;
};
