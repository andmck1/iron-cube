/**
 * @file
 * @brief Linux platform implementation using GLFW.
 */

#pragma once

#include "platform/IPlatform.h"
#include "platform/linux/WindowGLFW.h"
#include "platform/linux/InputGLFW.h"
#include <GLFW/glfw3.h>

/**
 * @brief Linux platform backend.
 *
 * Wraps GLFW for window management and keyboard input.
 * Exposes the raw GLFW window handle for the renderer layer.
 */
class PlatformLinux : public IPlatform
{
    public:
    bool initialise() override;
    bool createWindow(const PlatformConfig &config) override;
    void pollEvents() override;
    bool shouldExit() const override;
    void shutdown() override;

    /**
     * @brief Get the underlying GLFW window handle.
     * @return Pointer to the GLFWwindow, or nullptr if not created.
     *
     * Intended for the rendering layer to set up GL/VK contexts.
     */
    GLFWwindow *getGLFWWindow() const;

    private:
    WindowGLFW window_;
    InputGLFW input_;
    bool initialised_;
};
