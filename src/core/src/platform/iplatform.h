#pragma once

#include <string>
#include <cstdint>

struct PlatformConfig
{
    std::string window_title = "Iron Cube";
    uint32_t window_width = 1280;
    uint32_t window_height = 720;
};

class IPlatform
{
    public:
    virtual ~IPlatform() = default;

    virtual bool initialise() = 0;
    virtual bool createWindow(const PlatformConfig &config) = 0;
    virtual void pollEvents() = 0;
    virtual bool shouldExit() const = 0;
    virtual void shutdown() = 0;
};
