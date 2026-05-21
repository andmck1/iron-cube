#pragma once

#include "platform/IPlatform.h"
#include "platform/linux/WindowGLFW.h"
#include "platform/linux/InputGLFW.h"
#include <GLFW/glfw3.h>

class PlatformLinux : public IPlatform
{
    public:
    bool initialise() override;
    bool createWindow(const PlatformConfig &config) override;
    void pollEvents() override;
    bool shouldExit() const override;
    void shutdown() override;

    GLFWwindow *getGLFWWindow() const;

    private:
    WindowGLFW window_;
    InputGLFW input_;
    bool initialised_;
};
