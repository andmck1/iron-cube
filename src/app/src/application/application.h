#pragma once

#include "core_actor/camera.h"
#include "core_common/logger.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "core_layer/debug.h"
#include "core_layer/debuglayerdata.h"
#include "core_layer/scene.h"
#include "core_platform/linux/platform.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shadermanager.h"
#include "core_render/vulkan/shaderpipelinemanager.h"
#include "core_resource/resourcemanager.h"
#include "core_mesh/meshmanager.h"
#include "core_layer/ilayer.h"
#include "core_config/config.h"
#include <memory>
#include <vector>

static constexpr float MOUSE_SENSITIVITY = 1.5f;
static constexpr float MOVEMENT_SENSITIVITY = 5.f;
static constexpr uint32_t N_INSTANCE_COLUMNS = 40;
static constexpr uint32_t N_INSTANCE_ROWS = 40;
static constexpr float PIPELINE_DEBUG_QUERY_FREQUENCY = 1.f;

class Application
{
    public:
    bool init(int argc, char *argv[]);
    bool run();
    void shutdown();

    private:
    void initLogging();
    void initConfig();
    bool registerServices();
    bool initialisePlatform();
    bool initialiseRenderer();
    bool initialiseShaders();
    bool createAndInitialiseLayers();
    void initialiseDebugLayerData(Scene *scene);
    bool initialiseEntities(Scene *scene);
    bool
    initialiseGeometry(Scene *scene, std::string name, Transform transform);
    bool initialiseCamera(Scene *scene);
    void handleInput();
    // TODO - should this go somewhere else?
    void updateEntities();
    void mainLoop();

    void checkForPipelineUpdates();

    bool debugMode_ = false;
    Logger logger_;
    Config config_;
    PlatformConfig platformConfig_;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    ResourceManager resourceManager_;
    MeshManager meshManager_;
    ShaderManager shaderManager_;
    ShaderPipelineManager shaderPipelineManager_;
    CameraActor *camera_;
    SceneLayer *sceneLayer_;
    DebugLayer *debugLayer_;

    DebugLayerData debugLayerData_;
    Timer timer_;
    std::vector<std::unique_ptr<ILayer>> layers_;

    float timeSincePipelineDebugQuery_ = 0.f;
    int visibilityFunction_ = VisibilityFunc::VISIBILITY_FUNC_NONE;
    bool isAnimationOn_ = false;
};
