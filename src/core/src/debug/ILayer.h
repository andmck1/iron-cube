/**
 * @file
 * @brief Common layer interface for stack-based rendering.
 *
 * Defines the contract for renderable components in the layer
 * stack. Each layer implements initialise(), render(), and
 * shutdown() for lifecycle management.
 */

#pragma once

/**
 * @brief Layer interface for stack-based rendering.
 *
 * Abstract base for components that participate in the render
 * layer stack. The Application manages layer lifecycle, calling
 * initialise() once, render() each frame, and shutdown() on exit.
 */
class ILayer
{
    public:
    virtual ~ILayer() = default;

    /**
     * @brief Initialize the layer.
     * @return true on success.
     */
    virtual bool initialise() = 0;

    /**
     * @brief Render the layer for the current frame.
     *
     * Called once per frame in stack order. For ImGui-based
     * layers, this populates ImGui widgets. For scene layers,
     * this may be a no-op (rendering handled by VulkanRenderer).
     */
    virtual void render() = 0;

    /**
     * @brief Release layer resources.
     *
     * Called during Application teardown in reverse stack order.
     */
    virtual void shutdown() = 0;
};
