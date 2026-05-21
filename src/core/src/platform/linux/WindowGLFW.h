#pragma once

#include "platform/IWindow.h"
#include <GLFW/glfw3.h>
#include <cstdint>

class WindowGLFW : public IWindow
{
    public:
    bool init(uint32_t width, uint32_t height, const char *title);
    void destroy();

    bool isCloseRequested() const;
    bool isOpen() const override;
    bool isIconified() const override;
    void requestClose() override;
    Vec2u getSize() const override;

    GLFWwindow *getGLFWWindow() const;

    private:
    static void closeCallback(GLFWwindow *);

    GLFWwindow *window_;
};
