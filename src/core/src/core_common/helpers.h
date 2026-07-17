#pragma once
#include <random>
#include <string>
#include <vector>

struct GPUStats
{
    struct GPUMemory
    {
        float free;
        float used;
        float total;
        float percent_utilisation;
    };

    struct GPUUtilisation
    {
        unsigned int percent_kernel_executing;
        unsigned int percent_memoryrw_busy;
    };

    std::string name;
    GPUMemory memory;
    GPUUtilisation utilisation;
};

void printUsage(const std::string &progName);
std::vector<GPUStats> queryGPUStats();
std::string formatNumber(double value);

class Random
{
    public:
    Random();
    float randomFloat(float min, float max);

    private:
    std::random_device randomDevice_;
    std::mt19937 generator_;
};
