#include "Application.h"
#include <spdlog/spdlog.h>
#include "service/ServiceLocator.h"
#include "config/Config.h"
#include "layer/SceneLayer.h"
#include "layer/DebugLayer.h"

Application::Application()
{
}

void Application::initConfig()
{
    config_ = Config();
    config_.initConfig();
    platformConfig_ = config_.getPlatformConfig();
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

    if (!platform_.createWindow(platformConfig_))
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

void Application::initialiseDebugLayerData(Scene *scene)
{
    debugLayerData_.scene = scene;
}

int Application::createAndInitialiseLayers()
{
    layers_.push_back(std::make_unique<SceneLayer>());
    SceneLayer *sceneLayer = dynamic_cast<SceneLayer *>(layers_.back().get());

    initialiseDebugLayerData(&sceneLayer->scene);
    layers_.push_back(std::make_unique<DebugLayer>());
    DebugLayer *debugLayer = dynamic_cast<DebugLayer *>(layers_.back().get());
    debugLayer->setDebugData(&debugLayerData_);

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

void Application::initialiseEntities()
{
    spdlog::info("Initialising entities");
}

void Application::mainLoop()
{
    spdlog::info("Starting main loop");
    while (!platform_.shouldExit())
    {
        platform_.pollEvents();

        if (!renderer_.beginFrame())
            return;

        if (!renderer_.newFrame())
            return;

        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->render();
        }

        if (!renderer_.renderImGui())
            return;

        if (!renderer_.endFrame())
            return;
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
