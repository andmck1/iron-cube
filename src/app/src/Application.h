/**
 * @file
 * @brief Application entry point and lifecycle management.
 */

#pragma once

#include "platform/linux/PlatformLinux.h"
#include "rendering/linux/VulkanRenderer.h"
#include "scene/Scene.h"
#include "resource/ResourceManager.h"
#include "debug/ILayer.h"
#include "config/Config.h"
#include <memory>
#include <vector>

/**
 * @brief Top-level application class.
 *
 * Orchestrates platform initialisation, renderer setup,
 * the main event/render loop, and clean shutdown.
 *
 * @par Usage
 * @code
 * Application app;
 * return app.run();
 * @endcode
 */
class Application
{
    public:
    Application();

    /**
     * @brief Run the application main loop.
     * @return 0 on normal exit, non-zero on error.
     */
    int run();

    private:
    void initConfig();
    int registerServices();
    int initialisePlatform();
    int initialiseScene();
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
