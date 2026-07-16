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
#include "layer/scene.h"
#include "layer/debug.h"
#include "core_common/helpers.h"

#include "shaderpipeline/standard.h"
#include "shaderpipeline/instancedVertex.h"
#include "shaderpipeline/instancedVertexWithCompute.h"
#include "shaderpipeline/instancedMesh.h"

#include <format>
#include <iostream>
#include <spdlog/spdlog.h>
#include <algorithm>

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

void Application::initialiseShader(std::string path, std::string name)
{
    VulkanShaderConfig config;
    config.path = path;
    std::unique_ptr<VulkanShader> shader =
        std::make_unique<VulkanShader>(config);
    shaderManager_.registerShader(name, std::move(shader));
}

bool Application::initialiseShaders()
{
    spdlog::debug("Initialising shaders");

    // TODO - load these from config
    initialiseShader("build/shader/simpleVF/simple.vert.spv", "simpleVFVertex");
    initialiseShader(
        "build/shader/simpleVF/simple.frag.spv", "simpleVFFragment"
    );

    initialiseShader(
        "build/shader/instancedVF/instanced.vert.spv", "instancedVFVertex"
    );
    initialiseShader(
        "build/shader/instancedVF/instanced.frag.spv", "instancedVFFragment"
    );

    initialiseShader(
        "build/shader/instancedVFCompute/lodStats.comp.spv",
        "instancedVFCLODStats"
    );
    initialiseShader(
        "build/shader/instancedVFCompute/drawCommand.comp.spv",
        "instancedVFCDrawCommand"
    );
    initialiseShader(
        "build/shader/instancedVFCompute/instancedCompute.vert.spv",
        "instancedVFCVertex"
    );
    initialiseShader(
        "build/shader/instancedVFCompute/instancedCompute.frag.spv",
        "instancedVFCFragment"
    );

    initialiseShader(
        "build/shader/instancedMesh/instancedMesh.task.spv", "instancedMeshTask"
    );
    initialiseShader(
        "build/shader/instancedMesh/instancedMesh.mesh.spv", "instancedMeshMesh"
    );
    initialiseShader(
        "build/shader/instancedMesh/instancedMesh.frag.spv",
        "instancedMeshFragment"
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
    debugLayerData_.nInstanceRows = &nInstanceRows_;
    debugLayerData_.nInstanceColumns = &nInstanceColumns_;
    debugLayerData_.nInstanceLayers = &nInstanceLayers_;
    debugLayerData_.cameraFarPlane = &camera_->farClip_;
    debugLayerData_.isDebugStatsOn = &isDebugQueryOn_;
    debugLayerData_.gpuStats = &gpuStats_;
    debugLayerData_.lodIndex = &lodIndex_;
    debugLayerData_.maxLODDistance = &maxLODDistance_;
    debugLayerData_.lodExponent = &lodExponent_;
    debugLayerData_.isLODOn = &isLODOn_;
    debugLayerData_.fragmentColourType = &fragmentColourType;
}

bool Application::createAndInitialiseLayers()
{
    layers_.push_back(std::make_unique<SceneLayer>());
    sceneLayer_ = dynamic_cast<SceneLayer *>(layers_.back().get());

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
    initialiseDebugLayerData(&sceneLayer_->scene);

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
            {"assets/horse_statue_lod/horse_statue_01_4k_LOD_0.obj",
             "assets/horse_statue_lod/horse_statue_01_4k_LOD_1.obj",
             "assets/horse_statue_lod/horse_statue_01_4k_LOD_2.obj",
             "assets/horse_statue_lod/horse_statue_01_4k_LOD_3.obj",
             "assets/horse_statue_lod/horse_statue_01_4k_LOD_4.obj"},
            "./assets/horse_statue_lod"
        );
        resourceManager_.loadGeometry(name, geometry);
        // Transform transform = Transform{
        //     .position = {0.f, -1.f, 0.f}, .scale = {10.f, 10.f, 10.f}
        // };
        // initialiseGeometry(scene, name, transform);
    }

    {
        // asteroid
        std::string name = "asteroid";
        Geometry geometry = ResourceManager::loadObj(
            {"assets/asteroid_lod/asteroid_LOD_10.obj",
             "assets/asteroid_lod/asteroid_LOD_11.obj",
             "assets/asteroid_lod/asteroid_LOD_12.obj",
             "assets/asteroid_lod/asteroid_LOD_13.obj",
             "assets/asteroid_lod/asteroid_LOD_14.obj"},
            "./assets/asteroid_lod"
        );
        resourceManager_.loadGeometry(name, geometry);

        Texture texture =
            ResourceManager::loadImage("assets/asteroid/texture/diffuse.jpg");
        resourceManager_.loadTexture("asteroid", texture);
        Transform transform =
            Transform{.position = {0.f, -1.f, 0.f}, .scale = {1.f, 1.f, 1.f}};
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
    initialiseEntities();
    // updateEntities();

    // transform component
    TransformComponent *transformComponent =
        actorPtr->getComponent<TransformComponent>();
    transformComponent->setTransform(transform);

    // mesh component
    MeshComponent *meshComponent = actorPtr->getComponent<MeshComponent>();

    // TODO - generalise
    {
        // vertex shader pipeline
        std::unique_ptr<DefaultShaderPipelineUpdateData> updateData =
            std::make_unique<DefaultShaderPipelineUpdateData>();
        updateData->actor = actorPtr;
        updateData->camera = camera_;
        updateData->lodIndex = &lodIndex_;
        updateData->isAnimationOn = &isAnimationOn_;
        updateData->nFramesSinceStart = &nFramesSinceStart_;
        updateData->rotationFrequency = &rotationFrequency_;
        updateData->timer = &timer_;
        updateData->texture = resourceManager_.getTexture(name);
        updateData->fragmentColourType = &fragmentColourType;

        std::unique_ptr<StandardVulkanShaderPipeline> actorShaderPipeline =
            std::make_unique<StandardVulkanShaderPipeline>();
        auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
            shaderPipelineManager_.registerShaderPipeline(
                std::format("{}DefaultVertex", name),
                std::move(actorShaderPipeline), std::move(updateData)
            );

        registeredShaderPipeline->init(
            registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
        );
        meshComponent->setShaderPipeline(registeredShaderPipeline);
        meshComponent->setMesh(registeredMesh);
    }
    {
        // instanced vertex shader pipeline
        std::unique_ptr<InstancedVertexShaderPipelineUpdateData> updateData =
            std::make_unique<InstancedVertexShaderPipelineUpdateData>();
        updateData->actor = actorPtr;
        updateData->camera = camera_;
        updateData->instances = &registeredMesh->instances;
        updateData->lodIndex = &lodIndex_;
        updateData->isAnimationOn = &isAnimationOn_;
        updateData->rotationFrequency = &rotationFrequency_;
        updateData->timer = &timer_;
        updateData->texture = resourceManager_.getTexture(name);
        updateData->fragmentColourType = &fragmentColourType;

        std::unique_ptr<InstancedVertexVulkanShaderPipeline>
            actorInstancedShaderPipeline =
                std::make_unique<InstancedVertexVulkanShaderPipeline>();
        auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
            shaderPipelineManager_.registerShaderPipeline(
                std::format("{}InstancedVertex", name),
                std::move(actorInstancedShaderPipeline), std::move(updateData)
            );

        registeredShaderPipeline->init(
            registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
        );
    }
    {
        // instanced vertex shader pipeline
        std::unique_ptr<InstancedVertexComputeShaderPipelineUpdateData>
            updateData = std::make_unique<
                InstancedVertexComputeShaderPipelineUpdateData>();
        updateData->actor = actorPtr;
        updateData->camera = camera_;
        updateData->instances = &registeredMesh->instances;
        updateData->lodIndex = &lodIndex_;
        updateData->isAnimationOn = &isAnimationOn_;
        updateData->rotationFrequency = &rotationFrequency_;
        updateData->timer = &timer_;
        updateData->visibilityFunc = &visibilityFunction_;
        updateData->maxLODDistance = &maxLODDistance_;
        updateData->lodExponent = &lodExponent_;
        updateData->isLODOn = &isLODOn_;
        updateData->texture = resourceManager_.getTexture(name);
        updateData->fragmentColourType = &fragmentColourType;

        std::unique_ptr<InstancedVertexComputeVulkanShaderPipeline>
            actorInstancedShaderPipeline =
                std::make_unique<InstancedVertexComputeVulkanShaderPipeline>();
        auto [registeredShaderPipeline, registeredShaderPipelineUpdateData] =
            shaderPipelineManager_.registerShaderPipeline(
                std::format("{}InstancedVertexCompute", name),
                std::move(actorInstancedShaderPipeline), std::move(updateData)
            );

        registeredShaderPipeline->init(
            registeredMesh->getGeometry(), registeredShaderPipelineUpdateData
        );
    }
    {
        // mesh shader pipeline
        std::unique_ptr<InstancedMeshShaderPipelineUpdateData> updateData =
            std::make_unique<InstancedMeshShaderPipelineUpdateData>();
        updateData->actor = actorPtr;
        updateData->camera = camera_;
        updateData->instances = &registeredMesh->instances;
        updateData->visibilityFunc = &visibilityFunction_;
        updateData->maxLODDistance = &maxLODDistance_;
        updateData->lodExponent = &lodExponent_;
        updateData->isLODOn = &isLODOn_;
        updateData->isAnimationOn = &isAnimationOn_;
        updateData->rotationFrequency = &rotationFrequency_;
        updateData->timer = &timer_;
        updateData->texture = resourceManager_.getTexture(name);
        updateData->fragmentColourType = &fragmentColourType;

        std::unique_ptr<InstancedMeshVulkanShaderPipeline>
            actorMeshShaderPipeline =
                std::make_unique<InstancedMeshVulkanShaderPipeline>();
        auto
            [registeredMeshShaderPipeline,
             registeredMeshShaderPipelineUpdateData] =
                shaderPipelineManager_.registerShaderPipeline(
                    std::format("{}Mesh", name),
                    std::move(actorMeshShaderPipeline), std::move(updateData)
                );
        // TODO - should init reference the pointer to this update data?
        registeredMeshShaderPipeline->init(
            registeredMesh->getGeometry(),
            registeredMeshShaderPipelineUpdateData
        );
    }

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
    transformComponent->moveTo(0.f, 0.f, -3.f);
    transformComponent->rotateBy(0.f, 0.f, 0.f);

    scene->addEntity(std::move(camera));
    camera_ = dynamic_cast<CameraActor *>(scene->getEntities().back().get());

    spdlog::debug("Finished initialising camera entity");
    return true;
}

void Application::handleInput()
{
    // Clamp delta time to prevent rotation/movement spikes after frame
    // stalls
    float deltaSeconds = std::min(timer_.getDeltaSeconds(), 0.1f);
    float movementSpeed = MOVEMENT_SENSITIVITY * deltaSeconds;

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

        // Mouse delta is already frame-rate-independent (raw movement), so
        // don't multiply by delta time - just apply sensitivity directly
        rotationOffset.y -= delta.x * MOUSE_SENSITIVITY;
        rotationOffset.x = delta.y * MOUSE_SENSITIVITY;

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
        checkForEntitiyUpdates();
        checkForPipelineUpdates();
        shaderPipelineManager_.checkForUpdates();

        renderer_.beginFrame();

        // updateEntities();
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

        nFramesSinceStart_++;
    }
}

void Application::checkForEntitiyUpdates()
{
    bool checkRows = currentNInstanceRows_ != nInstanceRows_;
    bool checkColumns = currentNInstanceColumns_ != nInstanceColumns_;
    bool checkLayers = currentNInstanceLayers_ != nInstanceLayers_;
    if (checkRows || checkColumns || checkLayers)
        updateEntities();
    currentNInstanceRows_ = nInstanceRows_;
    currentNInstanceColumns_ = nInstanceColumns_;
    currentNInstanceLayers_ = nInstanceLayers_;
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
            shaderPipelineSuffix = "DefaultVertex";
            break;
        }
        case 1:
        {
            shaderPipelineSuffix = "InstancedVertex";
            break;
        }
        case 2:
        {
            shaderPipelineSuffix = "InstancedVertexCompute";
            break;
        }
        case 3:
        {
            shaderPipelineSuffix = "Mesh";
            break;
        }
        default:
        {
            shaderPipelineSuffix = "DefaultVertex";
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

    for (const std::unique_ptr<Entity> &entity : entities)
    {
        MeshComponent *meshComponent = entity->getComponent<MeshComponent>();
        if (meshComponent)
        {
            IShaderPipeline *shaderPipeline =
                meshComponent->getShaderPipeline();
            if (shaderPipeline)
            {
                shaderPipeline->setIsDebugQuery(isDebugQueryOn_);
            }
        }
    }

    timeSinceGPUStatsQuery_ += timer_.getDeltaSeconds();
    if (isDebugQueryOn_)
    {
        if (timeSinceGPUStatsQuery_ > 1.0 / GPU_STATS_QUERY_FREQUENCY)
        {
            timeSinceGPUStatsQuery_ = 0.f;
            gpuStats_ = queryGPUStats();
        }
    }
}

void Application::initialiseEntities()
{
    Mesh *mesh = meshManager_.getMesh("asteroid");
    mesh->instances.resize(
        nInstanceColumns_ * nInstanceLayers_ * nInstanceRows_
    );
    float maxSpan = std::max<float>(
        std::max<float>(
            mesh->getGeometry()->meshBounds.getWidth(),
            mesh->getGeometry()->meshBounds.getDepth()
        ),
        mesh->getGeometry()->meshBounds.getHeight()
    );
    float instanceSpanX = 2.0f * maxSpan;
    float instanceSpanY = 2.0f * maxSpan;
    float instanceSpanZ = 2.0f * maxSpan;
    float totalSpanX = nInstanceColumns_ * instanceSpanX;
    float totalSpanY = nInstanceLayers_ * instanceSpanY;
    float totalSpanZ = nInstanceRows_ * instanceSpanZ;

    for (uint32_t rowIndex = 0; rowIndex < nInstanceRows_; ++rowIndex)
    {
        for (uint32_t layerIndex = 0; layerIndex < nInstanceLayers_;
             ++layerIndex)
        {
            for (uint32_t columnIndex = 0; columnIndex < nInstanceColumns_;
                 ++columnIndex)
            {
                float x = columnIndex * instanceSpanX - (totalSpanX / 2.f) +
                          instanceSpanX / 2.f;
                float y = layerIndex * instanceSpanY - (totalSpanY / 2.f) +
                          instanceSpanY / 2.f;
                float z = rowIndex * instanceSpanZ - (totalSpanZ / 2.f) +
                          instanceSpanZ / 2.f;

                uint32_t index =
                    (layerIndex * nInstanceRows_ * nInstanceColumns_) +
                    (rowIndex * nInstanceColumns_) + columnIndex;
                float t = static_cast<float>(glfwGetTime()) +
                          ((columnIndex ^ rowIndex + columnIndex) / 10.f);
                glm::vec4 rotationVector;
                if (isAnimationOn_)
                    rotationVector = glm::vec4(0.0, t, 0.0, 1.0);
                else
                    rotationVector = glm::vec4(0.0, 0.0, 0.0, 1.0);

                mesh->instances[index].position = glm::vec4(x, y, z, 1.0);
                mesh->instances[index].rotation = rotationVector;
                mesh->instances[index].scale = glm::vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
    }
}

// TODO - seems super hardcoded all of this - I think there might be an
// animation component and I add a callback when initialising the horse
// actor and then call animation update in the scene layer frame
void Application::updateEntities()
{
    Mesh *mesh = meshManager_.getMesh("asteroid");
    mesh->instances.resize(
        nInstanceColumns_ * nInstanceLayers_ * nInstanceRows_
    );
    float maxSpan = std::max<float>(
        std::max<float>(
            mesh->getGeometry()->meshBounds.getWidth(),
            mesh->getGeometry()->meshBounds.getDepth()
        ),
        mesh->getGeometry()->meshBounds.getHeight()
    );
    float instanceSpanX = 2.0f * maxSpan;
    float instanceSpanY = 2.0f * maxSpan;
    float instanceSpanZ = 2.0f * maxSpan;
    float totalSpanX = nInstanceColumns_ * instanceSpanX;
    float totalSpanY = nInstanceLayers_ * instanceSpanY;
    float totalSpanZ = nInstanceRows_ * instanceSpanZ;

    for (uint32_t rowIndex = 0; rowIndex < nInstanceRows_; ++rowIndex)
    {
        for (uint32_t layerIndex = 0; layerIndex < nInstanceLayers_;
             ++layerIndex)
        {
            for (uint32_t columnIndex = 0; columnIndex < nInstanceColumns_;
                 ++columnIndex)
            {
                float x = columnIndex * instanceSpanX - (totalSpanX / 2.f) +
                          instanceSpanX / 2.f;
                float y = layerIndex * instanceSpanY - (totalSpanY / 2.f) +
                          instanceSpanY / 2.f;
                float z = rowIndex * instanceSpanZ - (totalSpanZ / 2.f) +
                          instanceSpanZ / 2.f;

                uint32_t index =
                    (layerIndex * nInstanceRows_ * nInstanceColumns_) +
                    (rowIndex * nInstanceColumns_) + columnIndex;
                float t = static_cast<float>(glfwGetTime()) +
                          ((columnIndex ^ rowIndex + columnIndex) / 10.f);
                glm::vec4 rotationVector;
                if (isAnimationOn_)
                    rotationVector = glm::vec4(0.0, t, 0.0, 1.0);
                else
                    rotationVector = glm::vec4(0.0, 0.0, 0.0, 1.0);

                mesh->instances[index].position = glm::vec4(x, y, z, 1.0);
                mesh->instances[index].rotation = rotationVector;
                mesh->instances[index].scale = glm::vec4(1.0, 1.0, 1.0, 1.0);
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
