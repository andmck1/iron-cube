/**
 * @file
 * @brief Rendering layer interface.
 *
 * The renderer owns the graphics context, framebuffer management,
 * and present/swap operations, keeping rendering concerns separate
 * from the platform layer.
 */

#pragma once

#include <GLFW/glfw3.h>

/**
 * @brief Renderer interface.
 *
 * Abstracts graphics backend initialization, frame presentation,
 * and cleanup. Implementations may use OpenGL, Vulkan, or other APIs.
 */
class IRenderer
{
    public:
    virtual ~IRenderer() = default;

    /**
     * @brief Initialize the renderer.
     * @param window Platform window handle for context creation.
     * @return true on success.
     */
    virtual bool init(GLFWwindow *window) = 0;

    /**
     * @brief Render the current frame.
     *
     * Records and submits draw commands. Reads scene data from IScene service.
     * Call once per frame before swapBuffers().
     */
    virtual void render() = 0;

    /**
     * @brief Present the current frame and swap front/back buffers.
     *
     * Call once per frame after rendering is complete.
     */
    virtual void swapBuffers() = 0;

    /**
     * @brief Release all renderer resources.
     *
     * Destroys the graphics context and frees GPU resources.
     */
    virtual void shutdown() = 0;
};
