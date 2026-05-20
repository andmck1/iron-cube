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
     * @brief Initialize and generate scene geometry data.
     * @return true on success.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Return reference to vertex data array.
     * @return Const reference to the vertex vector.
     */
    virtual const std::vector<Vertex>& getVertices() const = 0;

    /**
     * @brief Return number of vertices in scene.
     * @return Vertex count.
     */
    virtual uint32_t getVertexCount() const = 0;

    /**
     * @brief Check if scene data has changed since last render.
     * @return true if dirty.
     */
    virtual bool isDirty() const = 0;

    /**
     * @brief Mark scene data as clean after rendering.
     */
    virtual void markClean() = 0;

    /**
     * @brief Clear all scene data and release resources.
     */
    virtual void shutdown() = 0;
};
