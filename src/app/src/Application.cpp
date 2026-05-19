#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"

Application::Application()
{
}

int Application::run()
{
    spdlog::info("Iron Cube starting");

    auto &locator = ServiceLocator::instance();
    locator.registerService<IPlatform>(&platform_);
    locator.registerService<IRenderer>(&renderer_);

    if (!platform_.initialize())
    {
        spdlog::error("Platform initialization failed");
        return 1;
    }

    PlatformConfig config;
    config.window_title = "Iron Cube";
    config.window_width = 1280;
    config.window_height = 720;

    if (!platform_.createWindow(config))
    {
        spdlog::error("Window creation failed");
        platform_.shutdown();
        return 1;
    }

    if (!renderer_.init(platform_.getGLFWWindow()))
    {
        spdlog::error("Renderer initialization failed");
        platform_.shutdown();
        return 1;
    }

    while (!platform_.shouldExit())
    {
        platform_.pollEvents();
        renderer_.swapBuffers();
    }

    renderer_.shutdown();
    platform_.shutdown();
    spdlog::info("Iron Cube shutting down");
    return 0;
}
