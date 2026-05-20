/**
 * @file
 * @brief Application entry point and lifecycle management.
 */

#pragma once

#include "platform/linux/PlatformLinux.h"
#include "rendering/linux/VulkanRenderer.h"
#include "scene/Scene.h"
#include "resource/ResourceManager.h"
#include "service/ServiceLocator.h"

/**
 * @brief Top-level application class.
 *
 * Orchestrates platform initialization, renderer setup,
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
    PlatformLinux platform_;
    VulkanRenderer renderer_;
    Scene scene_;
    ResourceManager resourceManager_;
};
