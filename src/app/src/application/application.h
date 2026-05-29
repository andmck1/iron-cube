#pragma once

#include "core_common/logger.h"
#include "core_layer/debuglayerdata.h"
#include "core_platform/linux/platform.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shaderpipelinemanager.h"
#include "core_resource/resourcemanager.h"
#include "core_mesh/meshmanager.h"
#include "core_layer/ilayer.h"
#include "core_config/config.h"
#include <memory>
#include <vector>

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
    bool createAndInitialiseLayers();
    void initialiseDebugLayerData(Scene *scene);
    bool initialiseEntities(Scene *scene);
    void mainLoop();

    bool debugMode_ = false;
    Logger logger_;
    Config config_;
    PlatformConfig platformConfig_;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    ResourceManager resourceManager_;
    MeshManager meshManager_;
    ShaderPipelineManager shaderPipelineManager_;
    DebugLayerData debugLayerData_;
    std::vector<std::unique_ptr<ILayer>> layers_;
};
