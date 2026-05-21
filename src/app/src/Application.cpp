#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"
#include "debug/SceneLayer.h"
#include "debug/DebugLayer.h"

Application::Application()
{
}

int Application::registerServices()
{
    auto &locator = ServiceLocator::instance();
    locator.registerService<IPlatform>(&platform_);
    locator.registerService<IRenderer>(&renderer_);
    locator.registerService<IScene>(&scene_);
    locator.registerService<IResourceManager>(&resourceManager_);
    return 0;
}

int Application::initializePlatform()
{
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

    return 0;
}

int Application::initializeScene()
{
    scene_.initialize();
    return 0;
}

int Application::initializeRenderer()
{
    if (!renderer_.init(platform_.getGLFWWindow()))
    {
        spdlog::error("Renderer initialization failed");
        platform_.shutdown();
        return 1;
    }

    renderer_.setExitRequestedCallback([&]()
                                       { return platform_.shouldExit(); });

    return 0;
}

int Application::createAndInitializeLayers()
{
    layers_.push_back(std::make_unique<SceneLayer>(&renderer_));
    layers_.push_back(std::make_unique<DebugLayer>());

    for (auto &layer : layers_)
    {
        if (!layer->initialize())
        {
            spdlog::error("Layer initialization failed");
            renderer_.shutdown();
            platform_.shutdown();
            return 1;
        }
    }

    return 0;
}

void Application::mainLoop()
{
    while (!platform_.shouldExit())
    {
        platform_.pollEvents();

        renderer_.beginFrame();
        renderer_.newFrame();

        for (auto &layer : layers_)
        {
            layer->render();
        }

        renderer_.renderImGui();
        renderer_.endFrame();
    }
}

void Application::shutdownLayers()
{
    for (auto &layer : layers_)
    {
        layer->shutdown();
    }
    layers_.clear();
}

void Application::shutdownServices()
{
    scene_.shutdown();
    renderer_.shutdown();
    platform_.shutdown();
}

void Application::shutdownAll()
{
    shutdownLayers();
    shutdownServices();
}

int Application::run()
{
    spdlog::info("Iron Cube starting");

    try
    {
        if (registerServices() != 0)
            return 1;
        if (initializePlatform() != 0)
            return 1;
        if (initializeScene() != 0)
            return 1;
        if (initializeRenderer() != 0)
            return 1;
        if (createAndInitializeLayers() != 0)
            return 1;

        mainLoop();
        shutdownAll();
        return 0;
    }
    catch (const std::runtime_error &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        shutdownAll();
        return 1;
    }
}
