#pragma once

#include "core_common/loggerservice.h"
#include "core_layer/debuglayerdata.h"
#include "core_platform/linux/platformlinux.h"
#include "core_render/linux/vulkanrenderer.h"
#include "core_resource/resourcemanager.h"
#include "core_material/materialservice.h"
#include "core_mesh/meshservice.h"
#include "core_layer/ilayer.h"
#include "core_config/config.h"
#include <memory>
#include <vector>

class Application
{
    public:
    bool init(int argc, char *argv[]);
    bool run();

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
    void shutdownLayers();
    void shutdownServices();
    void shutdownAll();

    bool debugMode_ = false;
    LoggerService logger_;
    Config config_;
    PlatformConfig platformConfig_;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    ResourceManager resourceManager_;
    MaterialService materialService_;
    MeshesService meshService_;
    DebugLayerData debugLayerData_;
    std::vector<std::unique_ptr<ILayer>> layers_;
};
