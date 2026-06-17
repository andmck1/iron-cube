#include "application.h"
#include "core_actor/sceneactor.h"
#include "core_ecs/component/transform.h"
#include "core_ecs/component/mesh.h"
#include "core_render/ishaderpipelinemanager.h"
#include "core_render/vulkan/shader.h"
#include "core_platform/iinput.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "core_mesh/mesh.h"
#include "core_service/servicelocator.h"
#include "core_config/config.h"
#include "core_layer/scene.h"
#include "core_layer/debug.h"
#include "core_common/helpers.h"

#include "shaderpipeline/standard.h"
#include "shaderpipeline/instancedMesh.h"

#include <format>
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

    VulkanShaderConfig defaultTaskShaderConfig;
    defaultTaskShaderConfig.path = "build/shader/slang/task/default.task.spv";

    std::unique_ptr<VulkanShader> defaultTaskShader =
        std::make_unique<VulkanShader>(defaultTaskShaderConfig);
    shaderManager_.registerShader("defaultTask", std::move(defaultTaskShader));

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
    debugLayerData_.visibilityFunc = &visibilityFunction_;
    debugLayerData_.isAnimationOn = &isAnimationOn_;
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

    {
        // horsie
        std::string name = "horsie";
        Geometry geometry = ResourceManager::loadObj(
            "assets/horse_statue/horse_statue_01_4k.obj",
            "./assets/horse_statue"
        );
        resourceManager_.loadGeometry(name, geometry);
        Transform transform = Transform{
            .position = {0.f, -1.f, 0.f}, .scale = {10.f, 10.f, 10.f}
        };
        initialiseGeometry(scene, name, transform);
    }

    spdlog::info("Finished initilising entities");
    return true;
}

bool Application::initialiseGeometry(
    Scene *scene, std::string name, Transform transform
)
{
    spdlog::debug("Initialising entity: {}", name);

    // entity
    std::unique_ptr<SceneActor> actor = std::make_unique<SceneActor>(name);
    scene->addEntity(std::move(actor));
    SceneActor *actorPtr =
        dynamic_cast<SceneActor *>(scene->getEntities().back().get());

    // mesh
    Mesh actorMesh = Mesh();
    actorMesh.setGeometry(name);
    Mesh *registeredMesh =
        meshManager_.registerMesh(name, std::move(actorMesh));
    registeredMesh->instances.resize(
        N_INSTANCE_COLUMNS * N_INSTANCE_LAYERS * N_INSTANCE_ROWS
    );
    updateEntities();

    // transform component
    TransformComponent *transformComponent =
        actorPtr->getComponent<TransformComponent>();
    transformComponent->setTransform(transform);

    // mesh component
    MeshComponent *meshComponent = actorPtr->getComponent<MeshComponent>();

    // vertex shader pipeline
    std::unique_ptr<DefaultShaderPipelineUpdateData>
        actorShaderPipelineUpdateData =
            std::make_unique<DefaultShaderPipelineUpdateData>();
    actorShaderPipelineUpdateData->actor = actorPtr;
    actorShaderPipelineUpdateData->camera = camera_;

    std::unique_ptr<StandardVulkanShaderPipeline> actorShaderPipeline =
        std::make_unique<StandardVulkanShaderPipeline>();
    auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
        shaderPipelineManager_.registerShaderPipeline(
            std::format("{}Traditional", name), std::move(actorShaderPipeline),
            std::move(actorShaderPipelineUpdateData)
        );

    registeredShaderPipeline->init(
        registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
    );
    meshComponent->setShaderPipeline(registeredShaderPipeline);
    meshComponent->setMesh(registeredMesh);

    // mesh shader pipeline
    std::unique_ptr<InstancedMeshShaderPipelineUpdateData>
        actorMeshShaderPipelineUpdateData =
            std::make_unique<InstancedMeshShaderPipelineUpdateData>();
    actorMeshShaderPipelineUpdateData->actor = actorPtr;
    actorMeshShaderPipelineUpdateData->camera = camera_;
    actorMeshShaderPipelineUpdateData->instances = &registeredMesh->instances;
    actorMeshShaderPipelineUpdateData->visibilityFunc = &visibilityFunction_;

    std::unique_ptr<InstancedMeshVulkanShaderPipeline> actorMeshShaderPipeline =
        std::make_unique<InstancedMeshVulkanShaderPipeline>();
    auto
        [registeredMeshShaderPipeline, registeredMeshShaderPipelineUpdateData] =
            shaderPipelineManager_.registerShaderPipeline(
                std::format("{}Mesh", name), std::move(actorMeshShaderPipeline),
                std::move(actorMeshShaderPipelineUpdateData)
            );
    // TODO - should init reference the pointer to this update data?
    registeredMeshShaderPipeline->init(
        registeredMesh->getGeometry(), registeredMeshShaderPipelineUpdateData
    );

    spdlog::debug("Finished initialising triangle entity");
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
        // app updates
        timer_.tick();
        platform_.pollEvents();
        handleInput();

        // entity updates
        checkForPipelineUpdates();
        shaderPipelineManager_.checkForUpdates();
        renderer_.beginFrame();
        updateEntities();
        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->update();
        }

        // render
        renderer_.beginRender();
        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->frame();
        }

        renderer_.endRender();

        // post render updates
        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->postRender();
        }

        renderer_.drawFrame();
        renderer_.present();

        for (std::unique_ptr<ILayer> &layer : layers_)
        {
            layer->end();
        }
    }
}

void Application::checkForPipelineUpdates()
{
    IShaderPipelineManager *shaderPipelineManager =
        ServiceLocator::instance().get<IShaderPipelineManager>();
    const std::vector<std::unique_ptr<Entity>> &entities =
        sceneLayer_->scene.getEntities();
    if (debugLayer_->shaderTypeUpdate > -1)
    {
        std::string shaderPipelineSuffix = "";
        switch (debugLayer_->shaderTypeUpdate)
        {
        case 0:
        {
            shaderPipelineSuffix = "Traditional";
            break;
        }
        case 1:
        {
            shaderPipelineSuffix = "Mesh";
            break;
        }
        default:
        {
            shaderPipelineSuffix = "Traditional";
            break;
        }
        }
        for (const std::unique_ptr<Entity> &entity : entities)
        {
            MeshComponent *meshComponent =
                entity->getComponent<MeshComponent>();
            if (meshComponent)
            {
                IShaderPipeline *shaderPipeline = nullptr;
                shaderPipeline = shaderPipelineManager->getShaderPipeline(
                    std::format("{}{}", entity->getName(), shaderPipelineSuffix)
                );
                if (shaderPipeline)
                {
                    MeshComponent *meshComp =
                        entity->getComponent<MeshComponent>();
                    if (meshComp)
                        meshComp->setShaderPipeline(shaderPipeline);
                }
            }
        }
    }

    timeSincePipelineDebugQuery_ += timer_.getDeltaSeconds();

    if (timeSincePipelineDebugQuery_ >= (1.f / PIPELINE_DEBUG_QUERY_FREQUENCY))
    {
        timeSincePipelineDebugQuery_ = 0.f;
        for (const std::unique_ptr<Entity> &entity : entities)
        {
            MeshComponent *meshComponent =
                entity->getComponent<MeshComponent>();
            if (meshComponent)
            {
                IShaderPipeline *shaderPipeline =
                    meshComponent->getShaderPipeline();
                if (shaderPipeline)
                {
                    shaderPipeline->setIsDebugQuery(true);
                }
            }
        }
    }
}

// TODO - seems super hardcoded all of this - I think there might be an
// animation component and I add a callback when initialising the horse actor
// and then call animation update in the scene layer frame
void Application::updateEntities()
{
    Mesh *horseMesh = meshManager_.getMesh("horsie");
    float maxSpan = std::max<float>(
        std::max<float>(
            horseMesh->getGeometry()->meshBounds.getWidth(),
            horseMesh->getGeometry()->meshBounds.getDepth()
        ),
        horseMesh->getGeometry()->meshBounds.getHeight()
    );
    float instanceSpanX = 2.0f * maxSpan;
    float instanceSpanY = 2.0f * maxSpan;
    float instanceSpanZ = 2.0f * maxSpan;
    float totalSpanX = N_INSTANCE_COLUMNS * instanceSpanX;
    float totalSpanY = N_INSTANCE_LAYERS * instanceSpanY;
    float totalSpanZ = N_INSTANCE_ROWS * instanceSpanZ;

    for (uint32_t rowIndex = 0; rowIndex < N_INSTANCE_ROWS; ++rowIndex)
    {
        for (uint32_t layerIndex = 0; layerIndex < N_INSTANCE_LAYERS;
             ++layerIndex)
        {
            for (uint32_t columnIndex = 0; columnIndex < N_INSTANCE_COLUMNS;
                 ++columnIndex)
            {
                float x = columnIndex * instanceSpanX - (totalSpanX / 2.f) +
                          instanceSpanX / 2.f;
                float y = layerIndex * instanceSpanY - (totalSpanY / 2.f) +
                          instanceSpanY / 2.f;
                float z = rowIndex * instanceSpanZ - (totalSpanZ / 2.f) +
                          instanceSpanZ / 2.f;

                uint32_t index =
                    (layerIndex * N_INSTANCE_COLUMNS) +
                    (rowIndex * N_INSTANCE_LAYERS * N_INSTANCE_COLUMNS) +
                    columnIndex;
                float t = static_cast<float>(glfwGetTime()) +
                          ((columnIndex ^ rowIndex + columnIndex) / 10.f);
                glm::mat4x4 rotationMatrix;
                if (isAnimationOn_)
                    rotationMatrix =
                        glm::rotate(glm::mat4(1.f), t, glm::vec3(0, 1, 0));
                else
                    rotationMatrix = glm::identity<glm::mat4x4>();

                horseMesh->instances[index].transform =
                    glm::translate(glm::mat4(1.f), glm::vec3(x, y, z)) *
                    rotationMatrix;
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
