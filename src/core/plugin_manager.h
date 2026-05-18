// src/core/plugin_manager.h
#pragma once

#include <vector>
#include <memory>
#include "interface_renderer_configurator.h" // Assuming this is now in src/contracts/
#include "central_manager.h"

/**
 * @brief Manages the lifecycle and registration of all active plugins using
 * IRenderConfigurator.
 */
class PluginManager
{
    public:
    // Initializes and validates provided list of plugins against
    // CentralCoreManager contract.
    void loadAndValidatePlugins(
        CentralCoreManager &manager,
        const std::vector<std::shared_ptr<IRenderConfigurator>> &plugins
    );

    private:
    // Could hold internal state like plugin version database or manifest file
    // path
};