#include "platform/linux/windowglfw.h"
#include <spdlog/spdlog.h>

GLFWwindow *WindowGLFW::getGLFWWindow() const
{
    return window_;
}

bool WindowGLFW::init(uint32_t width, uint32_t height, const char *title)
{
    window_ = glfwCreateWindow(
        static_cast<int>(width), static_cast<int>(height), title, nullptr,
        nullptr
    );
    if (!window_)
    {
        spdlog::error("Failed to create window");
        return false;
    }

    glfwSetWindowCloseCallback(window_, closeCallback);
    glfwMakeContextCurrent(window_);

    return true;
}

void WindowGLFW::destroy()
{
    if (window_)
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

bool WindowGLFW::isCloseRequested() const
{
    if (!window_)
        return false;
    return glfwWindowShouldClose(window_) == GLFW_TRUE;
}

bool WindowGLFW::isOpen() const
{
    return window_ != nullptr && !isCloseRequested();
}

bool WindowGLFW::isIconified() const
{
    if (!window_)
        return false;
    return glfwGetWindowAttrib(window_, GLFW_ICONIFIED) == GLFW_TRUE;
}

void WindowGLFW::requestClose()
{
    if (window_)
    {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
}

Vec2u WindowGLFW::getSize() const
{
    if (!window_)
        return {0, 0};
    int w = 0, h = 0;
    glfwGetWindowSize(window_, &w, &h);
    return {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
}

void WindowGLFW::closeCallback(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
