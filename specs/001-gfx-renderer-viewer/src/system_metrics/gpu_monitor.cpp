// src/system_metrics/gpu_monitor.cpp
#include "gpu_monitor.h"
#include <cstdlib> // For system() or popen()

using namespace SystemMetrics;

bool GpuMonitor::initialize(const std::string &gpu_tool)
{
    m_gpu_tool = gpu_tool;
    LOGGER_INFO("GPU Monitor initialized using tool: {}", m_gpu_tool);
    return true; // Assume success for scaffold
}

// T040 Implementation: Executes system commands to gather live hardware
// metrics.
bool GpuMonitor::readMetrics(GraphicsStatisticsSnapshot &snapshot)
{
    std::string command =
        m_gpu_tool + " --query-gpu=clocks,utilization,vram,memory --format=csv";

    // Use popen or system() to execute the external tool and capture output.
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        LOGGER_ERROR("Failed to execute GPU monitoring command.");
        return false;
    }

    char buffer[128];
    // Read all lines from the pipeline... (Implementation detail skipped for
    // scaffold) For now, we mock a successful read.
    snapshot = {}; // Reset snapshot
    snapshot.gpu_utilization_ % = 45.0f;
    snapshot.vram_usage_bytes = 32 * 1024 * 1024;
    snapshot.ram_usage_bytes = 8 * 1024 * 1024 * 1024;
    LOGGER_INFO("Mock GPU metrics read successfully.");

    return true;
}