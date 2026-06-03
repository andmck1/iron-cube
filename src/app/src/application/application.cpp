#include "application.h"
#include "core_ecs/component/transform.h"
#include "core_ecs/component/mesh.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_render/vulkan/shader.h"
#include "glm/ext/vector_float3.hpp"
#include "core_mesh/mesh.h"
#include "core_service/servicelocator.h"
#include "core_config/config.h"
#include "core_layer/scene.h"
#include "core_layer/debug.h"
#include "core_common/helpers.h"

#include "shaderpipeline/default.h"

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
    Logger logger_ = Logger();
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
    locator.registerService<IMeshManager>(&meshManager_);
    locator.registerService<IShaderManager>(&shaderManager_);
    locator.registerService<IShaderPipelineManager>(&shaderPipelineManager_);
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

    return true;
}

bool Application::initialiseShaders()
{
    spdlog::debug("Initialising shaders");

    // TODO - look up nicer init styles
    VulkanShaderConfig defaultVertexShaderConfig;
    defaultVertexShaderConfig.path =
        "build/shader/slang/vertex/default.vert.spv";

    std::unique_ptr<VulkanShader> defaultVertexShader =
        std::make_unique<VulkanShader>(defaultVertexShaderConfig);
    shaderManager_.registerShader(
        "defaultVertex", std::move(defaultVertexShader)
    );

    VulkanShaderConfig defaultFragmentShaderConfig;
    defaultFragmentShaderConfig.path =
        "build/shader/slang/fragment/default.frag.spv";

    std::unique_ptr<VulkanShader> defaultFragmentShader =
        std::make_unique<VulkanShader>(defaultFragmentShaderConfig);
    shaderManager_.registerShader(
        "defaultFragment", std::move(defaultFragmentShader)
    );

    spdlog::debug("Finished initialising shaders");
    return true;
}

bool Application::initialiseShaderPipelines()
{
    std::unique_ptr<DefaultVulkanShaderPipeline> triangleShaderPipeline =
        std::make_unique<DefaultVulkanShaderPipeline>();
    auto _ = shaderPipelineManager_.registerShaderPipeline(
        "triangle", std::move(triangleShaderPipeline)
    );

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

    // initialiseDebugLayerData(&sceneLayer->scene);
    // layers_.push_back(std::make_unique<DebugLayer>());
    // DebugLayer *debugLayer = dynamic_cast<DebugLayer
    // *>(layers_.back().get()); debugLayer->setDebugData(&debugLayerData_);

    for (std::unique_ptr<ILayer> &layer : layers_)
    {
        if (!layer->initialise())
        {
            spdlog::error("Layer initialisation failed");
            platform_.shutdown();
            return false;
        }
    }
    if (!initialiseEntities(&sceneLayer->scene))
        return false;

    return true;
}

bool Application::initialiseEntities(Scene *scene)
{
    spdlog::info("Initialising entities");

    // triangle
    std::shared_ptr<std::vector<VertexInputType>> triangleVertices =
        ResourceManager::generateProceduralTriangle();
    resourceManager_.loadGeometry(
        "triangle", triangleVertices,
        static_cast<uint32_t>(triangleVertices->size())
    );
    
    // Confusing API as triangle mesh is dangling
    Mesh triangleMesh = Mesh();
    triangleMesh.setGeometry("triangle");
    Mesh *registeredMesh =
        meshManager_.registerMesh("triangle", std::move(triangleMesh));

    std::unique_ptr<Entity> triangle = std::make_unique<Entity>("triangle");

    TransformComponent *transformComponent =
        triangle->addComponent<TransformComponent>();
    transformComponent->setScale(glm::vec3(1.f, 1.f, 1.f));
    transformComponent->setRotation(glm::vec3(0.f, 0.f, 0.f));
    transformComponent->setPosition(glm::vec3(1.f, 1.f, 1.f));

    MeshComponent *meshComponent = triangle->addComponent<MeshComponent>();
    IShaderPipeline *triangleShaderPipeline =
        shaderPipelineManager_.getShaderPipeline("triangle");
    triangleShaderPipeline->init(registeredMesh->getGeometry());
    meshComponent->setShaderPipeline(triangleShaderPipeline);
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
        platform_.pollEvents();

        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->frame();
        }
    }
}

void Application::shutdown()
{
    spdlog::info("Shutting application down");
    renderer_.shutdown();
    platform_.shutdown();
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
        if (!initialiseShaders())
            return false;
        if (!initialiseShaderPipelines())
            return false;
        if (!createAndInitialiseLayers())
            return false;

        mainLoop();
        shutdown();
        spdlog::info("Iron Cube shut down cleanly");
        return true;
    }
    catch (const std::runtime_error &e)
    {
        spdlog::error("Fatal error: {}", e.what());
        shutdown();
        return false;
    }
}
