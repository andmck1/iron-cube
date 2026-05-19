#include "rendering/linux/RendererGLFW.h"

bool RendererGLFW::init(GLFWwindow *window)
{
    window_ = window;
    glfwMakeContextCurrent(window_);
    return window_ != nullptr;
}

void RendererGLFW::swapBuffers()
{
    if (window_)
    {
        glfwSwapBuffers(window_);
    }
}

void RendererGLFW::shutdown()
{
    window_ = nullptr;
}
