#pragma once

#include <string>
#include "platform/IPlatform.h"
#include "yaml-cpp/node/node.h"

class Config
{
    public:
    Config() = default;

    int initConfig();
    PlatformConfig getPlatformConfig() const;

    private:
    std::string path_ = "";
    YAML::Node rootConfig;
};
