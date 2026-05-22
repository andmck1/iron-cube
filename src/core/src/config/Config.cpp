#include "config/Config.h"
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

bool Config::initConfig()
{
    try
    {
        rootConfig_ = YAML::LoadFile(path_);
    }
    catch (const YAML::Exception &e)
    {
        spdlog::error("Failed to parse config file {}: {}", path_, e.what());
        return false;
    }
    return true;
}

PlatformConfig Config::getPlatformConfig() const
{
    PlatformConfig config;

    try
    {
        YAML::Node platform = rootConfig_["platform"];

        if (platform)
        {
            if (platform["window_title"])
                config.window_title =
                    platform["window_title"].as<std::string>();
            if (platform["window_width"])
                config.window_width = platform["window_width"].as<uint32_t>();
            if (platform["window_height"])
                config.window_height = platform["window_height"].as<uint32_t>();
        }
        else
        {
            spdlog::warn(
                "No 'platform' section in config file {}, using defaults", path_
            );
        }
    }
    catch (const YAML::Exception &e)
    {
        spdlog::error("Failed to parse config file {}: {}", path_, e.what());
    }

    return config;
}
