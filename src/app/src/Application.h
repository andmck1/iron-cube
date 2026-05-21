#pragma once

#include "layers/DebugLayerData.h"
#include "platform/linux/PlatformLinux.h"
#include "rendering/linux/VulkanRenderer.h"
#include "resource/ResourceManager.h"
#include "layers/ILayer.h"
#include "config/Config.h"
#include <memory>
#include <vector>

class Application
{
    public:
    Application();

    int run();

    private:
    void initConfig();
    int registerServices();
    int initialisePlatform();
    int initialiseRenderer();
    void initialiseDebugLayerData(Scene *scene);
    int createAndInitialiseLayers();
    void mainLoop();
    void shutdownLayers();
    void shutdownServices();
    void shutdownAll();

    Config config_;
    PlatformConfig platformConfig_;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    ResourceManager resourceManager_;
    DebugLayerData debugLayerData_;
    std::vector<std::unique_ptr<ILayer>> layers_;
};
