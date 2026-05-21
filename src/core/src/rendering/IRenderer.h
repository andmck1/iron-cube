/**
 * @file
 * @brief Rendering layer interface.
 *
 * The renderer owns the graphics context, framebuffer management,
 * and present/swap operations, keeping rendering concerns separate
 * from the platform layer.
 */

#pragma once

#include <vulkan/vulkan.h>
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

    /**
     * @brief Initialize ImGui context and Vulkan backend.
     *
     * Creates ImGui context, initializes Vulkan backend with
     * device/queue, and creates font texture. Call after
     * swap chain setup during init().
     * @return true on success.
     */
    virtual bool initImGui() = 0;

    /**
     * @brief Start a new ImGui frame.
     *
     * Calls backend new frame setup and ImGui::NewFrame().
     * Call at start of each render cycle before scene rendering.
     */
    virtual void newFrame() = 0;

    /**
     * @brief Render ImGui draw data.
     *
     * Records ImGui draw commands into current frame's command buffer.
     * Call after layer render() returns and before swapBuffers().
     */
    virtual void renderImGui() = 0;

    /**
     * @brief Clean up ImGui Vulkan resources.
     *
     * Destroys descriptor pool, pipeline, font texture, and ImGui context.
     * Call before shutdown() during teardown.
     */
    virtual void shutdownImGui() = 0;
};
