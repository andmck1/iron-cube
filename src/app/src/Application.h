#pragma once

#include "platform/linux/PlatformLinux.h"
#include "rendering/linux/VulkanRenderer.h"
#include "scene/Scene.h"
#include "resource/ResourceManager.h"
#include "debug/ILayer.h"
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
    int createAndInitialiseLayers();
    void mainLoop();
    void shutdownLayers();
    void shutdownServices();
    void shutdownAll();

    Config config;
    PlatformConfig platformConfig;
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    Scene scene_;
    ResourceManager resourceManager_;
    std::vector<std::unique_ptr<ILayer>> layers_;
};
