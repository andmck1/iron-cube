#pragma once

#include "layer/debuglayerdata.h"
#include "platform/linux/platformlinux.h"
#include "render/linux/vulkanrenderer.h"
#include "resource/resourcemanager.h"
#include "material/materialsservice.h"
#include "mesh/meshesservice.h"
#include "layer/ilayer.h"
#include "config/config.h"
#include <memory>
#include <vector>

class Application
{
    public:
    Application();

    bool run();

    private:
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

    Config config_;
    PlatformConfig platformConfig_;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    ResourceManager resourceManager_;
    MaterialsService materialsService_;
    MeshesService meshesService_;
    DebugLayerData debugLayerData_;
    std::vector<std::unique_ptr<ILayer>> layers_;
};
