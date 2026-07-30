#pragma once

#include "commontypes.h"
#include "core_actor/camera.h"
#include "core_common/helpers.h"
#include "core_common/logger.h"
#include "core_common/timer.h"
#include "core_common/types.h"
#include "layer/debug.h"
#include "layer/debuglayerdata.h"
#include "layer/scene.h"
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

static constexpr float MOUSE_SENSITIVITY = 0.0015f;
static constexpr float MOVEMENT_SENSITIVITY = 5.f;
static constexpr float GPU_STATS_QUERY_FREQUENCY = 1.f;
static constexpr float EPS = 1e-6f;
static constexpr float INSTANCE_SCALE_LOWER = 0.6;
static constexpr float INSTANCE_SCALE_UPPER = 1.4;

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
    void initialiseShader(std::string path, std::string name);
    bool initialiseShaders();
    bool createAndInitialiseLayers();
    void initialiseDebugLayerData(Scene *scene);
    bool initialiseEntities(Scene *scene);
    bool
    initialiseGeometry(Scene *scene, std::string name, Transform transform);
    bool initialiseCamera(Scene *scene);
    void handleInput();
    // TODO - should this go somewhere else?
    void initialiseEntities();
    void updateEntities();
    void mainLoop();

    void checkForPipelineUpdates();
    void checkForEntitiyUpdates();

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

    float timeSinceGPUStatsQuery_ = 0.f;
    VisibilityFunc visibilityFunction_ =
        VisibilityFunc::VISIBILITY_FUNC_CONE_AND_NEAR_PLANE;
    bool isAnimationOn_ = true;

    int nInstances_ = 3250000;
    float discOuterRadius_ = 100000.f;
    float discInnerRadius_ = 10.f;
    float discHeight_ = 1000.f;

    int currentNInstances_ = nInstances_;
    float currentDiscOuterRadius_ = discOuterRadius_;
    float currentDiscInnerRadius_ = discInnerRadius_;
    float currentDiscHeight_ = discHeight_;

    bool isDebugQueryOn_ = false;
    std::vector<GPUStats> gpuStats_ = {};
    uint32_t lodIndex_ = 0;
    float maxLODDistance_ = 100.f;
    float lodExponent_ = 5.f;
    bool isLODOn_ = true;
    uint32_t nFramesSinceStart_ = 0;
    float rotationFrequency_ = 0.01f;
    FragmentColourType fragmentColourType = FragmentColourType::TEXTURE;

    Random random = Random();
};
