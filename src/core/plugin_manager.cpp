// src/core/plugin_manager.cpp
#include "plugin_manager.h"
#include "../contracts/interface_renderer_configurator.h"
#include <iostream>

// Implementation of the Plugin Manager class (T050)
void PluginManager::loadAndValidatePlugins(
    CentralCoreManager &manager,
    const std::vector<std::shared_ptr<IRenderConfigurator>> &plugins
)
{
    for (const auto &plugin : plugins)
    {
        std::cout << "Attempting to initialize plugin: "
                  << plugin->getPluginID() << std::endl;
        if (plugin->initialize(manager))
        {
            std::cout << "Successfully initialized and registered plugin: "
                      << plugin->getPluginID() << std::endl;
            // Core Manager takes ownership of the active plugin pointer.
            // For simplicity here, we assume the manager handles registration
            // internally upon calling this method.
        }
        else
        {
            std::cerr << "WARNING: Failed to initialize plugin: "
                      << plugin->getPluginID() << ". Skipping." << std::endl;
        }
    }
}
