#pragma once
#define GLFW_INCLUDE_VULKAN

#include "core_platform/iplatform.h"
#include "core_platform/linux/windowglfw.h"
#include "core_platform/linux/inputglfw.h"
#include <GLFW/glfw3.h>

class PlatformLinux : public IPlatform
{
    public:
    ~PlatformLinux();
    bool initialise() override;
    bool createWindow(const PlatformConfig &config) override;
    void pollEvents() override;
    bool shouldExit() const override;
    void shutdown() override;
    const PlatformConfig &getConfig() const override;
    IInput *getInput() override;

    GLFWwindow *getGLFWWindow() const;

    private:
    WindowGLFW window_;
    InputGLFW input_;
    bool initialised_;
    PlatformConfig config_;
};
