#include "platform/linux/PlatformLinux.h"
#include <spdlog/spdlog.h>

bool PlatformLinux::initialise()
{
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (glfwInit())
    {
        initialised_ = true;
        return true;
    }
    spdlog::error("Failed to initialise GLFW");
    return false;
}

bool PlatformLinux::createWindow(const PlatformConfig &config)
{
    if (!initialised_)
    {
        spdlog::error("Platform not initialised");
        return false;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
    initialised_ = false;
}

GLFWwindow *PlatformLinux::getGLFWWindow() const
{
    return window_.getGLFWWindow();
}
