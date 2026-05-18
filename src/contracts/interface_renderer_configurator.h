// src/contracts/interface_renderer_configurator.h
#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>

/**
 * @brief The core interface for all third-party plugins to interact with the
 * Renderer's state. Must be implemented by any module wishing to configure
 * rendering parameters or hook into events.
 */
class IRenderConfigurator
{
    public:
    virtual ~IRenderConfigurator() = default;

    // Contract Validation: Must return a unique ID and version number.
    virtual std::string getPluginID() const = 0;
    virtual int getVersion() const = 0;

    /**
     * @brief Called by the Core Manager to initialize and validate plugin
     * resources (e.g., loading necessary shaders/textures).
     * @param core_manager A reference to the central system manager for access.
     * @return true if initialization was successful, false otherwise.
     */
    virtual bool initialize(class CentralCoreManager &core_manager) = 0;

    /**
     * @brief Attempts to set a rendering parameter value dynamically.
     * This method is mediated by the Core Manager for thread safety and
     * validation.
     * @param key The unique name of the setting (e.g., "triangle_color").
     * @param value The new value (must match expected type).
     * @return An optional string describing an error if the set operation fails
     * or is invalid.
     */
    virtual std::optional<std::string> setParameter(
        const std::string &key, const std::variant<float, bool> &value
    ) = 0;

    // Methods to query current state for UI/logging purposes
    virtual std::map<std::string, std::variant<float, bool>>
    getParameters() const = 0;
};
