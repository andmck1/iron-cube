// src/core/debug_settings.h
#pragma once

#include <map>
#include <string>
#include <variant>
#include <mutex>

/**
 * @brief Centralized, thread-safe storage for all runtime debug and rendering
 * parameters. This class ensures a single source of truth (SSOT) for mutable
 * state across plugins.
 */
class DebugSettings
{
    public:
    using SettingMap = std::map<std::string, std::variant<float, bool>>;

    DebugSettings(); // Default constructor required

    // Core methods remain the same...

    private:
    mutable std::mutex m_mutex;
    SettingMap m_settings;

    bool validateValue(
        const std::string &key, const std::variant<float, bool> &value
    ) const;
};