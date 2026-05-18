// src/ui/debug_panel.cpp
#include "debug_panel.h"
#include "../core/central_manager.h"

DebugPanel::DebugPanel(CentralCoreManager &core_manager)
    : m_core_manager(core_manager)
{
}

void DebugPanel::renderUI()
{
    // 1. Get current settings state from the manager (READ).
    auto current_settings = m_core_manager.getDebugSettings().getReadOnlyCopy();

    ImGui::Begin("Renderer Debug Controls");

    // 2. Color control example: Check for "triangle_color" and allow
    // modification.
    if (current_settings.count("triangle_color"))
    {
        ImGui::Text("Triangle Color (R, G, B):");
        float r = std::get<float>(current_settings.at("triangle_color"));
        // ... UI sliders for R, G, B ...

        // Mock change button to trigger the update flow
        if (ImGui::Button("Set Red"))
        {
            // 3. Attempt to write new state via the Core Manager API.
            // The manager handles validation and concurrency control
            // internally.
            std::variant<float, bool> new_val =
                1.0f; // Mock float value for red
            if (m_core_manager.getDebugSettings().setValue(
                    "triangle_color", new_val
                ))
            {
                Logger::log(
                    "Successfully set debug parameter 'triangle_color'."
                );
            }
            else
            {
                Logger::error(
                    "Failed to set debug parameter due to validation failure."
                );
            }
        }
    }

    // 4. Add other controls here (Vertex Scale, etc.)
    ImGui::End();
}