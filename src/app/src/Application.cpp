#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"
#include "debug/SceneLayer.h"
#include "debug/DebugLayer.h"

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

        // Create layer stack
        layers_.push_back(std::make_unique<SceneLayer>(&renderer_));
        layers_.push_back(std::make_unique<DebugLayer>());

        // Initialize layers
        for (auto& layer : layers_)
        {
            if (!layer->initialize())
            {
                spdlog::error("Layer initialization failed");
                renderer_.shutdown();
                platform_.shutdown();
                return 1;
            }
        }

        while (!platform_.shouldExit())
        {
            platform_.pollEvents();

            // Begin frame
            renderer_.beginFrame();
            renderer_.newFrame();

            // Render layers
            for (auto& layer : layers_)
            {
                layer->render();
            }

            // Render ImGui overlay
            renderer_.renderImGui();

            // End frame
            renderer_.endFrame();
        }

        // Shutdown layers
        for (auto& layer : layers_)
        {
            layer->shutdown();
        }
        layers_.clear();

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
