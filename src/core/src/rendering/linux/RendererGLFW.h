/**
 * @file
 * @brief GLFW renderer implementation for Linux.
 */

#pragma once

#include "rendering/IRenderer.h"

/**
 * @brief GLFW-based renderer backend.
 *
 * Manages the GLFW window's swap chain and framebuffer
 * presentation. A placeholder for the full rendering pipeline
 * (shaders, meshes, command buffers, etc.).
 */
class RendererGLFW : public IRenderer
{
    public:
    bool init(GLFWwindow *window) override;
    void swapBuffers() override;
    void shutdown() override;

    private:
    GLFWwindow *window_;
};
