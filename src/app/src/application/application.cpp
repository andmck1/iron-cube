#include "application.h"
#include "core_actor/sceneactor.h"
#include "core_ecs/component/transform.h"
#include "core_ecs/component/mesh.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_render/vulkan/shader.h"
#include "core_platform/iinput.h"
#include "glm/ext/vector_float3.hpp"
#include "core_mesh/mesh.h"
#include "core_service/servicelocator.h"
#include "core_config/config.h"
#include "core_layer/scene.h"
#include "core_layer/debug.h"
#include "core_common/helpers.h"

#include "shaderpipeline/default.h"
#include "shaderpipeline/defaultMesh.h"

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

    VulkanShaderConfig defaultMeshShaderConfig;
    defaultMeshShaderConfig.path = "build/shader/slang/mesh/default.mesh.spv";

    std::unique_ptr<VulkanShader> defaultMeshShader =
        std::make_unique<VulkanShader>(defaultMeshShaderConfig);
    shaderManager_.registerShader("defaultMesh", std::move(defaultMeshShader));

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

void Application::initialiseDebugLayerData(Scene *scene)
{
    debugLayerData_.scene = scene;
    debugLayerData_.timer = &timer_;
}

bool Application::createAndInitialiseLayers()
{
    layers_.push_back(std::make_unique<SceneLayer>());
    sceneLayer_ = dynamic_cast<SceneLayer *>(layers_.back().get());

    initialiseDebugLayerData(&sceneLayer_->scene);
    layers_.push_back(std::make_unique<DebugLayer>());
    debugLayer_ = dynamic_cast<DebugLayer *>(layers_.back().get());
    debugLayer_->setDebugData(&debugLayerData_);

    for (std::unique_ptr<ILayer> &layer : layers_)
    {
        if (!layer->initialise())
        {
            spdlog::error("Layer initialisation failed");
            platform_.shutdown();
            return false;
        }
    }
    if (!initialiseEntities(&sceneLayer_->scene))
        return false;

    return true;
}

bool Application::initialiseEntities(Scene *scene)
{
    spdlog::info("Initialising entities");
    // camera must be first for the view matrix
    initialiseCamera(scene);
    // initialiseCube(scene);
    initialiseTriangle(scene);
    spdlog::info("Finished initilising entities");
    return true;
}

// TODO - generalise for any geometry
bool Application::initialiseTriangle(Scene *scene)
{
    spdlog::debug("Initialising triangle entity");

    // entity
    std::unique_ptr<SceneActor> triangle =
        std::make_unique<SceneActor>("triangle");
    scene->addEntity(std::move(triangle));
    SceneActor *trianglePtr =
        dynamic_cast<SceneActor *>(scene->getEntities().back().get());

    // geometry
    Geometry triangleGeometry = ResourceManager::generateProceduralRectangle();
    resourceManager_.loadGeometry("triangle", triangleGeometry);

    // mesh
    Mesh triangleMesh = Mesh();
    triangleMesh.setGeometry("triangle");
    Mesh *registeredMesh =
        meshManager_.registerMesh("triangle", std::move(triangleMesh));

    // transform component
    auto _ = trianglePtr->getComponent<TransformComponent>();

    // mesh component
    MeshComponent *meshComponent = trianglePtr->getComponent<MeshComponent>();

    // vertex shader pipeline
    std::unique_ptr<DefaultShaderPipelineUpdateData>
        triangleShaderPipelineUpdateData =
            std::make_unique<DefaultShaderPipelineUpdateData>();
    triangleShaderPipelineUpdateData->actor = trianglePtr;
    triangleShaderPipelineUpdateData->camera = camera_;

    std::unique_ptr<DefaultVulkanShaderPipeline> triangleShaderPipeline =
        std::make_unique<DefaultVulkanShaderPipeline>();
    auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
        shaderPipelineManager_.registerShaderPipeline(
            "triangleTraditional", std::move(triangleShaderPipeline),
            std::move(triangleShaderPipelineUpdateData)
        );
    // TODO - should init reference the pointer to this update data?
    registeredShaderPipeline->init(
        registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
    );
    meshComponent->setShaderPipeline(registeredShaderPipeline);
    meshComponent->setMesh(registeredMesh);

    // mesh shader pipeline
    std::unique_ptr<DefaultMeshShaderPipelineUpdateData>
        triangleMeshShaderPipelineUpdateData =
            std::make_unique<DefaultMeshShaderPipelineUpdateData>();
    triangleMeshShaderPipelineUpdateData->actor = trianglePtr;
    triangleMeshShaderPipelineUpdateData->camera = camera_;

    std::unique_ptr<DefaultVulkanMeshShaderPipeline>
        triangleMeshShaderPipeline =
            std::make_unique<DefaultVulkanMeshShaderPipeline>();
    auto
        [registeredMeshShaderPipeline, registeredMeshShaderPipelineUpdateData] =
            shaderPipelineManager_.registerShaderPipeline(
                "triangleMesh", std::move(triangleMeshShaderPipeline),
                std::move(triangleMeshShaderPipelineUpdateData)
            );
    // TODO - should init reference the pointer to this update data?
    registeredMeshShaderPipeline->init(
        registeredMesh->getGeometry(), registeredMeshShaderPipelineUpdateData
    );

    spdlog::debug("Finished initialising triangle entity");
    return true;
}

// TODO - generalise for any geometry
bool Application::initialiseCube(Scene *scene)
{
    spdlog::debug("Initialising cube entity");

    // entity
    std::unique_ptr<SceneActor> cube = std::make_unique<SceneActor>("cube");
    scene->addEntity(std::move(cube));
    SceneActor *cubePtr =
        dynamic_cast<SceneActor *>(scene->getEntities().back().get());

    // geometry
    Geometry cubeGeometry = ResourceManager::generateProceduralCube();
    resourceManager_.loadGeometry("cube", cubeGeometry);

    // mesh
    Mesh cubeMesh = Mesh();
    cubeMesh.setGeometry("cube");
    Mesh *registeredMesh =
        meshManager_.registerMesh("cube", std::move(cubeMesh));

    // transform component
    auto _ = cubePtr->getComponent<TransformComponent>();

    // mesh component
    MeshComponent *meshComponent = cubePtr->getComponent<MeshComponent>();

    // shader pipeline
    std::unique_ptr<DefaultShaderPipelineUpdateData>
        cubeShaderPipelineUpdateData =
            std::make_unique<DefaultShaderPipelineUpdateData>();
    cubeShaderPipelineUpdateData->actor = cubePtr;
    cubeShaderPipelineUpdateData->camera = camera_;

    std::unique_ptr<DefaultVulkanShaderPipeline> cubeShaderPipeline =
        std::make_unique<DefaultVulkanShaderPipeline>();
    auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
        shaderPipelineManager_.registerShaderPipeline(
            "cube", std::move(cubeShaderPipeline),
            std::move(cubeShaderPipelineUpdateData)
        );
    // TODO - should init reference the pointer to this update data?
    registeredShaderPipeline->init(
        registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
    );

    meshComponent->setShaderPipeline(registeredShaderPipeline);
    meshComponent->setMesh(registeredMesh);

    spdlog::debug("Finished initialising cube entity");
    return true;
}

bool Application::initialiseCamera(Scene *scene)
{
    spdlog::debug("Initialising camera entity");

    // entity
    std::unique_ptr<CameraActor> camera =
        std::make_unique<CameraActor>("camera");

    // transform component
    TransformComponent *transformComponent =
        camera->getComponent<TransformComponent>();
    transformComponent->moveTo(0.f, 0.f, 3.f);
    transformComponent->rotateBy(0.f, 3.141f, 0.f);

    scene->addEntity(std::move(camera));
    camera_ = dynamic_cast<CameraActor *>(scene->getEntities().back().get());

    spdlog::debug("Finished initialising camera entity");
    return true;
}

void Application::handleInput()
{
    float deltaSeconds = timer_.getDeltaSeconds();
    float movementSpeed = MOVEMENT_SENSITIVITY * deltaSeconds;
    float rotationSpeed = MOUSE_SENSITIVITY * deltaSeconds;

    IInput *input = platform_.getInput();
    if (!input)
        return;
    TransformComponent *cameraTransform =
        camera_->getComponent<TransformComponent>();
    glm::vec3 rotationOffset = {0.f, 0.f, 0.f};
    glm::vec3 positionOffset = {0.f, 0.f, 0.f};

    if (input->isMouseButtonPressed(MouseButton::Right))
    {
        MouseDelta delta = input->getMouseDelta();

        rotationOffset.y -= delta.x * rotationSpeed;
        rotationOffset.x = delta.y * rotationSpeed;

        if (input->isKeyPressed(KeyCode::W))
            positionOffset.z += movementSpeed;
        if (input->isKeyPressed(KeyCode::S))
            positionOffset.z -= movementSpeed;
        if (input->isKeyPressed(KeyCode::D))
            positionOffset.x -= movementSpeed;
        if (input->isKeyPressed(KeyCode::A))
            positionOffset.x += movementSpeed;
        if (input->isKeyPressed(KeyCode::Q))
            positionOffset.y += movementSpeed;
        if (input->isKeyPressed(KeyCode::E))
            positionOffset.y -= movementSpeed;
    }

    cameraTransform->moveRelativeToForwardVectorBy(
        positionOffset.x, positionOffset.y, positionOffset.z
    );
    cameraTransform->rotateBy(
        rotationOffset.x, rotationOffset.y, rotationOffset.z
    );
    input->resetMouseDelta();
}

void Application::mainLoop()
{
    spdlog::info("Starting main loop");
    while (!platform_.shouldExit())
    {
        timer_.tick();
        platform_.pollEvents();
        handleInput();

        checkForPipelineUpdates();
        shaderPipelineManager_.checkForUpdates();
        renderer_.beginFrame();
        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->frame();
        }
        renderer_.drawFrame();
        renderer_.present();
    }
}

void Application::checkForPipelineUpdates()
{
    if (debugLayer_->shaderTypeUpdate > -1)
    {
        IShaderPipelineManager *shaderPipelineManager =
            ServiceLocator::instance().get<IShaderPipelineManager>();
        IShaderPipeline *shaderPipeline = nullptr;
        switch (debugLayer_->shaderTypeUpdate)
        {
        case 0:
        {
            shaderPipeline =
                shaderPipelineManager->getShaderPipeline("triangleTraditional");
            break;
        }
        case 1:
        {
            shaderPipeline =
                shaderPipelineManager->getShaderPipeline("triangleMesh");
            break;
        }
        default:
        {
            shaderPipeline =
                shaderPipelineManager->getShaderPipeline("triangleTraditional");
            break;
        }
        }
        if (shaderPipeline)
        {
            const std::vector<std::unique_ptr<Entity>> &entities =
                sceneLayer_->scene.getEntities();
            for (const std::unique_ptr<Entity> &entity : entities)
            {
                MeshComponent *meshComp = entity->getComponent<MeshComponent>();
                if (meshComp)
                    meshComp->setShaderPipeline(shaderPipeline);
            }
        }
    }
}

void Application::shutdown()
{
    spdlog::info("Shutting application down");
    renderer_.shutdown();
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
