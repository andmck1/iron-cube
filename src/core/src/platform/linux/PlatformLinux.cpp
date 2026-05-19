#include "platform/linux/PlatformLinux.h"
#include <spdlog/spdlog.h>

bool PlatformLinux::initialize()
{
    if (glfwInit())
    {
        initialized_ = true;
        return true;
    }
    spdlog::error("Failed to initialize GLFW");
    return false;
}

bool PlatformLinux::createWindow(const PlatformConfig &config)
{
    if (!initialized_)
    {
        spdlog::error("Platform not initialized");
        return false;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    if (!window_.init(
            config.window_width, config.window_height,
            config.window_title.c_str()
        ))
    {
        return false;
    }

    input_.init();
    glfwSetKeyCallback(window_.getGLFWWindow(), InputGLFW::keyCallback);

    glfwShowWindow(window_.getGLFWWindow());
    return true;
}

void PlatformLinux::pollEvents()
{
    glfwPollEvents();
}

bool PlatformLinux::shouldExit() const
{
    if (window_.isCloseRequested())
        return true;

    if (input_.isKeyPressed(KeyCode::Escape) && !window_.isIconified())
        return true;

    return false;
}

void PlatformLinux::shutdown()
{
    window_.destroy();
    glfwTerminate();
    initialized_ = false;
}

GLFWwindow *PlatformLinux::getGLFWWindow() const
{
    return window_.getGLFWWindow();
}
