/**
 * @file
 * @brief GLFW-based window implementation for Linux.
 */

#pragma once

#include "platform/IWindow.h"
#include <GLFW/glfw3.h>
#include <cstdint>

/**
 * @brief GLFW window backend.
 *
 * Manages the lifecycle of a single GLFW window, including
 * creation, destruction, and close callback handling.
 */
class WindowGLFW : public IWindow
{
    public:
    /**
     * @brief Create the window.
     * @param width  Window width in pixels.
     * @param height Window height in pixels.
     * @param title  Window title bar text.
     * @return true on success.
     */
    bool init(uint32_t width, uint32_t height, const char *title);

    /**
     * @brief Destroy the window and release resources.
     */
    void destroy();

    /**
     * @brief Check if the close flag was set (by user or callback).
     * @return true if close was requested.
     */
    bool isCloseRequested() const;

    bool isOpen() const override;
    bool isIconified() const override;
    void requestClose() override;
    Vec2u getSize() const override;

    /**
     * @brief Get the raw GLFW window handle.
     * @return Pointer to the GLFWwindow.
     */
    GLFWwindow *getGLFWWindow() const;

    private:
    static void closeCallback(GLFWwindow *);

    GLFWwindow *window_;
};
