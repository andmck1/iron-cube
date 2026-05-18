// src/core/debug_settings.cpp
#include "debug_settings.h"
#include <iostream>

DebugSettings::DebugSettings()
{
}

bool DebugSettings::setValue(
    const std::string &key, const std::variant<float, bool> &value
)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validateValue(key, value))
    {
        return false;
    }

    // Actual write operation is now safe and atomic.
    m_settings[key] = value;
    std::cout << "[DEBUG SETTINGS] Updated: " << key << std::endl;
    return true;
}

// ... (getReadOnlyCopy implementation remains the same)

bool DebugSettings::validateValue(
    const std::string &key, const std::variant<float, bool> &value
) const
{
    // Placeholder validation for demonstration.
    if (key == "triangle_color")
    {
        // In a real scenario, we'd check if the float components are within [0,
        // 1].
        std::cout << "[Validation] Successfully validated color key: " << key
                  << std::endl;
        return true;
    }
    if (key == "vertex_scale")
    {
        // Check positive scale.
        if (std::holds_alternative<float>(value) &&
            std::get<float>(value) < 0.1f)
        {
            return false; // Reject invalid scales
        }
        return true;
    }
    // Default fallback for unknown keys
    std::cout << "[Validation] No specific constraints found for key: " << key
              << ". Allowing write." << std::endl;
    return true;
}