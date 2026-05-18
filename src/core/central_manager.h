// src/core/central_manager.h
#pragma once

#include "../contracts/interface_renderer_configurator.h"
#include "debug_settings.h"
#include <map>
#include <memory>
#include <vector>

/**
 * @brief Orchestrates all core subsystems: Plugin lifecycle, State Management,
 * and Communication Routing.
 */
class CentralCoreManager
{
    public:
    CentralCoreManager();
    ~CentralCoreManager() = default;

    // --- Core Lifecycle Methods ---
    void initialize(
        const std::vector<std::shared_ptr<IRenderConfigurator>> &plugins
    );

    /**
     * @brief Registers a plugin, running its initialization logic.
     */
    bool registerPlugin(std::shared_ptr<IRenderConfigurator> plugin);

    // --- State Management Delegation ---
    DebugSettings &getDebugSettings()
    {
        return debug_settings_;
    }

    private:
    DebugSettings debug_settings_;
    // Tracks all active plugins by their unique ID
    std::map<std::string, std::shared_ptr<IRenderConfigurator>>
        registered_plugins_;
};
