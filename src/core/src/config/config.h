#pragma once

#include <string>
#include "platform/iplatform.h"
#include "yaml-cpp/node/node.h"

class Config
{
    public:
    Config() = default;

    bool initConfig();

    PlatformConfig getPlatformConfig() const;

    private:
    std::string path_ = "";
    YAML::Node rootConfig_ = {};
};
