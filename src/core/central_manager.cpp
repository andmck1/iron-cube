// src/core/central_manager.cpp
#include "central_manager.h"
#include "../util/serializer.hpp"
#include "../plugin_manager.h"

CentralCoreManager::CentralCoreManager() =
    default; // Initializes debug_settings_ and plugin_manager_

void CentralCoreManager::initialize(
    const std::vector<std::shared_ptr<IRenderConfigurator>> &plugins
)
{
    // 1. Initialize Plugins (T050)
    plugin_manager_->loadAndValidatePlugins(*this, plugins);

    // 2. Initial State Setup
    debug_settings_.setValue(
        "triangle_color", std::variant<float, bool>{1.0f}
    ); // Default Red
    debug_settings_.setValue(
        "vertex_scale", std::variant<float, bool>{1.0f}
    ); // Default scale
}

bool CentralCoreManager::registerPlugin(
    std::shared_ptr<IRenderConfigurator> plugin
)
{
    // Add plugin to internal map tracking (Placeholder for robust registry
    // management)
    registered_plugins_[plugin->getPluginID()] = plugin;
    return true;
}