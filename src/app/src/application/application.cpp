#include "application.h"
#include "core_ecs/component/transformcomponent.h"
#include "core_ecs/component/meshcomponent.h"
#include "glm/ext/vector_float3.hpp"
#include "core_mesh/mesh.h"
#include "core_service/servicelocator.h"
#include "core_config/config.h"
#include "core_layer/scenelayer.h"
#include "core_layer/debuglayer.h"
#include "core_common/helpers.h"

#include <csignal>
#include <atomic>

static std::atomic<bool> gSignalReceived{false};

static void signalHandler(int)
{
    gSignalReceived.store(true);
}

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
    LoggerService logger_ = LoggerService();
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
    locator.registerService<IMaterialService>(&materialService_);
    locator.registerService<IMeshService>(&meshService_);
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

    // TODO - this doesn't feel right. A material is too loosely defined here
    // And I don't know if each object has it's own graphics pipeline like this?
    // Maybe it does because each one is a unique shader pipeline and then you
    // apply that to a list of objects
    // Initialise renderer pipelines for all registered materials
    std::vector<Material *> allMaterials = materialService_.getAllMaterials();
    for (Material *material : allMaterials)
    {
        if (!renderer_.initialisePipeline(material->getConfig()))
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

    // triangle
    std::shared_ptr<std::vector<Vertex>> triangleVertices =
        ResourceManager::generateProceduralTriangle();
    resourceManager_.loadGeometry(
        "triangle", triangleVertices,
        static_cast<uint32_t>(triangleVertices->size())
    );

    Mesh triangleMesh = Mesh();
    triangleMesh.setGeometry("triangle");
    Mesh *registeredMesh =
        meshService_.registerMesh("triangle", std::move(triangleMesh));

    DefaultMaterial triangleMaterial = DefaultMaterial();
    Material *registeredMaterial = materialService_.registerMaterial(
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

    // --- Cube ---
    std::shared_ptr<std::vector<Vertex>> cubeVertices =
        ResourceManager::generateProceduralCube();
    resourceManager_.loadGeometry(
        "cube", cubeVertices, static_cast<uint32_t>(cubeVertices->size())
    );

    Mesh cubeMesh = Mesh();
    cubeMesh.setGeometry("cube");
    Mesh *registeredCubeMesh =
        meshService_.registerMesh("cube", std::move(cubeMesh));

    CubeMaterial cubeMaterial = CubeMaterial();
    Material *registeredCubeMaterial =
        materialService_.registerMaterial("cube", std::move(cubeMaterial));

    std::unique_ptr<Entity> cube = std::make_unique<Entity>("cube");

    TransformComponent *cubeTransformComponent =
        cube->addComponent<TransformComponent>();
    cubeTransformComponent->setScale(glm::vec3(1.f, 1.f, 1.f));
    cubeTransformComponent->setRotation(glm::vec3(0.f, 0.f, 0.f));
    cubeTransformComponent->setPosition(glm::vec3(-1.f, 0.f, 0.f));

    MeshComponent *cubeMeshComponent = cube->addComponent<MeshComponent>();
    cubeMeshComponent->setMaterial(registeredCubeMaterial);
    cubeMeshComponent->setMesh(registeredCubeMesh);
    scene->addEntity(std::move(cube));

    return true;
}

void Application::mainLoop()
{
    spdlog::info("Starting main loop");
    while (!platform_.shouldExit() && !gSignalReceived.load())
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

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

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
