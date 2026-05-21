#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"
#include "config/Config.h"
#include "debug/SceneLayer.h"
#include "debug/DebugLayer.h"

Application::Application()
{
}

void Application::initConfig()
{
    config = Config();
    config.initConfig();
    platformConfig = config.getPlatformConfig();
}

int Application::registerServices()
{
    ServiceLocator &locator = ServiceLocator::instance();
    locator.registerService<IPlatform>(&platform_);
    locator.registerService<IRenderer>(&renderer_);
    locator.registerService<IResourceManager>(&resourceManager_);
    return 0;
}

int Application::initialisePlatform()
{
    if (!platform_.initialise())
    {
        spdlog::error("Platform initialisation failed");
        return 1;
    }

    if (!platform_.createWindow(platformConfig))
    {
        spdlog::error("Window creation failed");
        platform_.shutdown();
        return 1;
    }

    return 0;
}

int Application::initialiseRenderer()
{
    if (!renderer_.init(platform_.getGLFWWindow()))
    {
        spdlog::error("Renderer initialisation failed");
        platform_.shutdown();
        return 1;
    }

    renderer_.setExitRequestedCallback([&]()
                                       { return platform_.shouldExit(); });

    return 0;
}

int Application::createAndInitialiseLayers()
{
    layers_.push_back(std::make_unique<SceneLayer>());
    SceneLayer *sceneLayer = dynamic_cast<SceneLayer *>(layers_.back().get());
    layers_.push_back(std::make_unique<DebugLayer>(&sceneLayer->scene));

    for (std::unique_ptr<ILayer> &layer : layers_)
    {
        if (!layer->initialise())
        {
            spdlog::error("Layer initialisation failed");
            renderer_.shutdown();
            platform_.shutdown();
            return 1;
        }
    }

    return 0;
}

void Application::mainLoop()
{
    spdlog::info("Starting main loop");
    while (!platform_.shouldExit())
    {
        platform_.pollEvents();

        renderer_.beginFrame();
        renderer_.newFrame();

        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->render();
        }

        renderer_.renderImGui();
        renderer_.endFrame();
    }
}

void Application::shutdownLayers()
{
    for (std::unique_ptr<ILayer> &layer : layers_)
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
        if (initialisePlatform() != 0)
            return 1;
        if (initialiseRenderer() != 0)
            return 1;
        if (createAndInitialiseLayers() != 0)
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
