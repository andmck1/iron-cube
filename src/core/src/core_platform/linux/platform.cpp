#include "core_platform/linux/platform.h"
#include "imgui_impl_glfw.h"
#include <spdlog/spdlog.h>

PlatformLinux::~PlatformLinux()
{
    shutdown();
}

bool PlatformLinux::initialise()
{
    spdlog::debug("Initialising platform (GLFW)");
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (glfwInit())
    {
        initialised_ = true;
        spdlog::debug("GLFW initialised successfully");
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

    spdlog::debug(
        "Creating window: {}x{} titled '{}'", config.window_width,
        config.window_height, config.window_title
    );
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    config_ = config;

    if (!window_.init(
            config.window_width, config.window_height,
            config.window_title.c_str()
        ))
    {
        spdlog::error("Window creation failed");
        return false;
    }

    input_.init();
    glfwSetKeyCallback(window_.getGLFWWindow(), InputGLFW::keyCallback);
    glfwSetMouseButtonCallback(
        window_.getGLFWWindow(), InputGLFW::mouseButtonCallback
    );
    glfwSetCursorPosCallback(
        window_.getGLFWWindow(), InputGLFW::cursorPositionCallback
    );

    glfwShowWindow(window_.getGLFWWindow());
    spdlog::debug("Window created and shown");
    return true;
}

void PlatformLinux::pollEvents()
{
    glfwPollEvents();
    ImGui_ImplGlfw_NewFrame();
}

bool PlatformLinux::shouldExit() const
{
    if (window_.isCloseRequested())
        return true;

    if (glfwWindowShouldClose(window_.getGLFWWindow()))
        return true;

    if (input_.isKeyPressed(KeyCode::Escape) && !window_.isIconified())
        return true;

    return false;
}

void PlatformLinux::shutdown()
{
    spdlog::debug("Platform shutting down");
    window_.destroy();
    glfwTerminate();
    initialised_ = false;
    spdlog::debug("Platform shut down complete");
}

GLFWwindow *PlatformLinux::getGLFWWindow() const
{
    return window_.getGLFWWindow();
}

const PlatformConfig &PlatformLinux::getConfig() const
{
    return config_;
}

IInput *PlatformLinux::getInput()
{
    return &input_;
}
