#include "core_common/helpers.h"

#include <nvml.h>
#include <iostream>
#include <sstream>

// AI help here
std::string formatNumber(double value)
{
    if (value < 0)
    {
        return "-" + formatNumber(-value);
    }

    std::string suffixes[] = {"", "k", "M", "B", "T"};
    int index = 0;

    double absValue = value;
    while (absValue >= 1000.0 &&
           index < (int) (sizeof(suffixes) / sizeof(suffixes[0]) - 1))
    {
        absValue /= 1000.0;
        index++;
    }

    std::ostringstream stream;
    if (absValue < 10.0)
    {
        stream << std::fixed;
        stream.precision(1);
        stream << absValue;
    }
    else
    {
        stream << std::fixed;
        stream.precision(0);
        stream << absValue;
    }

    return stream.str() + suffixes[index];
}

void printUsage(const std::string &progName)
{
    std::cerr << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  --debug    Enable debug-level logging\n"
              << "  --help     Show this help message\n";
}

// Some AI help here
// TODO - make this a class so I don't need to init and shutdown every time
// likely much faster and doesn't cost much frame time. Although, this means
// it's never on while debug query flag off
std::vector<GPUStats> queryGPUStats()
{
    std::vector<GPUStats> stats;
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS)
    {
        std::cerr << "Failed to initialise NVML\n";
        return stats;
    }

    unsigned int deviceCount;
    nvmlDeviceGetCount(&deviceCount);
    stats.resize(deviceCount);

    for (unsigned int i = 0; i < deviceCount; ++i)
    {
        nvmlDevice_t device;
        nvmlDeviceGetHandleByIndex(i, &device);
        char name[256];
        nvmlDeviceGetName(device, name, 256);
        stats[0].name = name;
        nvmlMemory_t memory;
        nvmlDeviceGetMemoryInfo(device, &memory);
        stats[0].memory.used =
            (float) memory.used / (float) (1024 * 1024 * 1024);
        stats[0].memory.free =
            (float) memory.free / (float) (1024 * 1024 * 1024);
        stats[0].memory.total =
            (float) memory.total / (float) (1024 * 1024 * 1024);
        stats[0].memory.percent_utilisation =
            (float) (memory.used) / (float) (memory.free + memory.used) * 100.0;
        nvmlUtilization_t utilisation;
        nvmlDeviceGetUtilizationRates(device, &utilisation);
        stats[0].utilisation.percent_kernel_executing = utilisation.gpu;
        stats[0].utilisation.percent_memoryrw_busy = utilisation.memory;
    }

    nvmlShutdown();

    return stats;
}

Random::Random()
{
    generator_ = std::mt19937(0);
}

float Random::randomFloat(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(generator_);
}
