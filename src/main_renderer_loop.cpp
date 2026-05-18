// src/main_renderer_loop.cpp
#include <iostream>
#include <memory>
#include "core/central_manager.h"
#include "renderer/vulkan_context.h"
#include "renderer/basic_pass.h"
#include "ui/debug_panel.h"
#include "system_metrics/gpu_monitor.hpp" // Include US2 Monitor

void runRendererApplication(WindowHandle window)
{
    // 1. SETUP CORE SYSTEMS (T050, T012, PluginManager::loadAndValidatePlugins)
    CentralCoreManager core_manager;
    // Mock plugin list for simulation
    std::vector<std::shared_ptr<IRenderConfigurator>> plugins;
    // In reality: Populate 'plugins' vector with real implementations (e.g.,
    // from PluginManager).
    core_manager.initialize(plugins); // Uses T050/T012

    // 2. INITIALIZE RENDERER (US1 - T020)
    VulkanContext renderer(window);
    if (!renderer.isDeviceInitialized())
    {
        std::cerr << "FATAL: Cannot run application without a functional "
                     "Vulkan context."
                  << std::endl;
        return;
    }

    // 3. SETUP DEBUGGING AND UI (US2 - T022)
    DebugPanel debug_panel(
        core_manager
    ); // Uses Central Core Manager state (T012)

    // --- MAIN APPLICATION LOOP (The Render Loop) ---
    const int MAX_FRAMES =
        5; // Simulate running for a few frames for demonstration
    for (int frame = 0; frame < MAX_FRAMES; ++frame)
    {
        std::cout << "\n--- FRAME " << frame + 1 << " START ---" << std::endl;

        // A. INPUT HANDLING & DEBUGGING: Render debug UI first, allowing state
        // changes. (US2/P1)
        debug_panel.renderUI();

        // B. RENDER PHASE (T021 - US1)
        BasicPass pass(core_manager);
        if (!pass.renderTriangle())
        {
            std::cerr << "Error during triangle rendering." << std::endl;
        }

        // C. STATS UPDATE (US3 - P2)
        SystemMetrics::GpuMonitor monitor;
        monitor.initialize("nvidia-smi"); // T040 call, US3
        GraphicsStatisticsSnapshot snapshot{};
        if (monitor.readMetrics(snapshot))
        { // Reads mock metrics for all components (US3)
            std::cout << "Frame Cycle Complete. Stats Captured: FPS="
                      << snapshot.fps
                      << ", GPUUtil=" << snapshot.gpu_utilization_ % << "%"
                      << std::endl;
        }
        else
        {
            std::cerr << "WARNING: Could not read system metrics this frame."
                      << std::endl;
        }
    }
}
