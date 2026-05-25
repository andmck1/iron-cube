#include "application.h"
#include "ecs/component/transformcomponent.h"
#include "ecs/component/meshcomponent.h"
#include "glm/ext/vector_float3.hpp"
#include "mesh/mesh.h"
#include "service/servicelocator.h"
#include "config/config.h"
#include "layer/scenelayer.h"
#include "layer/debuglayer.h"
#include "common/helpers.h"

#include <iostream>
#include <spdlog/spdlog.h>

bool Application::init(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--debug")
        {
            debugMode_ = true;
        }
        else if (arg == "--help")
        {
            printUsage(argv[0]);
            return false;
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return false;
        }
    }

    return true;
}

void Application::initLogging()
{
    logger_ = LoggerService();
    logger_.init(debugMode_);
}

void Application::initConfig()
{
    config_ = Config();
    config_.initConfig("config/config.yaml");
    platformConfig_ = config_.getPlatformConfig();
}

bool Application::registerServices()
{
    ServiceLocator &locator = ServiceLocator::instance();
    locator.registerService<ILogger>(&logger_);
    locator.registerService<IPlatform>(&platform_);
    locator.registerService<IRenderer>(&renderer_);
    locator.registerService<IResourceManager>(&resourceManager_);
    locator.registerService<IMaterialsService>(&materialsService_);
    locator.registerService<IMeshesService>(&meshesService_);
    return true;
}

bool Application::initialisePlatform()
{
    if (!platform_.initialise())
    {
        spdlog::error("Platform initialisation failed");
        return false;
    }

    if (!platform_.createWindow(platformConfig_))
    {
        spdlog::error("Window creation failed");
        platform_.shutdown();
        return false;
    }

    return true;
}

bool Application::initialiseRenderer()
{
    if (!renderer_.init(&platform_))
    {
        spdlog::error("Renderer initialisation failed");
        platform_.shutdown();
        return false;
    }

    renderer_.setExitRequestedCallback([&]()
                                       { return platform_.shouldExit(); });

    return true;
}

void Application::initialiseDebugLayerData(Scene *scene)
{
    debugLayerData_.scene = scene;
}

bool Application::createAndInitialiseLayers()
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
            return false;
        }
    }
    if (!initialiseEntities(&sceneLayer->scene))
        return false;

    // Initialise renderer pipelines now that materials are registered
    Material *triangleMaterial = materialsService_.getMaterial("triangle");
    if (triangleMaterial)
    {
        if (!renderer_.initialisePipelines(triangleMaterial->getPipeline()))
        {
            spdlog::error("Renderer pipeline initialisation failed");
            renderer_.shutdown();
            platform_.shutdown();
            return false;
        }
    }

    return true;
}

bool Application::initialiseEntities(Scene *scene)
{
    spdlog::info("Initialising entities");

    // Register procedural triangle geometry
    auto triangleVertices = ResourceManager::generateProceduralTriangle();
    resourceManager_.loadGeometry(
        "triangle", triangleVertices,
        static_cast<uint32_t>(triangleVertices->size())
    );

    Mesh triangleMesh = Mesh();
    triangleMesh.setGeometry("triangle");
    Mesh *registeredMesh =
        meshesService_.registerMesh("triangle", std::move(triangleMesh));

    Material triangleMaterial = Material();
    Material *registeredMaterial = materialsService_.registerMaterial(
        "triangle", std::move(triangleMaterial)
    );

    std::unique_ptr<Entity> triangle = std::make_unique<Entity>("triangle");

    TransformComponent *transformComponent =
        triangle->addComponent<TransformComponent>();
    transformComponent->setScale(glm::vec3(1.f, 1.f, 1.f));
    transformComponent->setRotation(glm::vec3(0.f, 0.f, 0.f));
    transformComponent->setPosition(glm::vec3(1.f, 1.f, 1.f));

    MeshComponent *meshComponent = triangle->addComponent<MeshComponent>();
    meshComponent->setMaterial(registeredMaterial);
    meshComponent->setMesh(registeredMesh);
    scene->addEntity(std::move(triangle));

    return true;
}

void Application::mainLoop()
{
    spdlog::info("Starting main loop");
    while (!platform_.shouldExit())
    {
        debugLayerData_.timer.tick();
        renderer_.setDeltaTime(debugLayerData_.timer.getDeltaSeconds());
        platform_.pollEvents();

        if (!renderer_.beginFrame())
            return;

        if (!renderer_.newFrame())
            return;

        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->render();
        }

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

bool Application::run()
{
    spdlog::info("Iron Cube starting");

    initLogging();
    initConfig();

    try
    {
        if (!registerServices())
            return false;
        if (!initialisePlatform())
            return false;
        if (!initialiseRenderer())
            return false;
        if (!createAndInitialiseLayers())
            return false;

        mainLoop();
        shutdownAll();
        spdlog::info("Iron Cube shut down cleanly");
        return true;
    }
    catch (const std::runtime_error &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        shutdownAll();
        return false;
    }
}
