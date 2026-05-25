#include "config/config.h"
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

bool Config::initConfig(const std::string &path)
{
    try
    {
        YAML::Node rootConfig = YAML::LoadFile(path);

        YAML::Node platform = rootConfig["platform"];

        if (platform)
        {
            if (platform["window_title"])
                platformConfig_.window_title =
                    platform["window_title"].as<std::string>();
            if (platform["window_width"])
                platformConfig_.window_width =
                    platform["window_width"].as<uint32_t>();
            if (platform["window_height"])
                platformConfig_.window_height =
                    platform["window_height"].as<uint32_t>();
        }
        else
        {
            spdlog::warn(
                "No 'platform' section in config file {}, using defaults", path
            );
        }
    }
    catch (const YAML::Exception &e)
    {
        spdlog::error("Failed to parse config file {}: {}", path, e.what());
        return false;
    }
    return true;
}

PlatformConfig Config::getPlatformConfig() const
{
    return platformConfig_;
}
