#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"

Application::Application()
{
}

int Application::run()
{
    spdlog::info("Iron Cube starting");

    try
    {
        auto &locator = ServiceLocator::instance();
        locator.registerService<IPlatform>(&platform_);
        locator.registerService<IRenderer>(&renderer_);
        locator.registerService<IScene>(&scene_);
        locator.registerService<IResourceManager>(&resourceManager_);

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

        scene_.initialize();

        if (!renderer_.init(platform_.getGLFWWindow()))
        {
            spdlog::error("Renderer initialization failed");
            platform_.shutdown();
            return 1;
        }

        renderer_.setExitRequestedCallback([&]() {
            return platform_.shouldExit();
        });

        while (!platform_.shouldExit())
        {
            platform_.pollEvents();
            renderer_.render();
            renderer_.swapBuffers();
        }
        
        scene_.shutdown();
        renderer_.shutdown();
        platform_.shutdown();
        return 0;
    }
    catch (const std::runtime_error& e)
    {
        spdlog::error("Fatal error: {}", e.what());
        scene_.shutdown();
        renderer_.shutdown();
        platform_.shutdown();
        return 1;
    }
}
