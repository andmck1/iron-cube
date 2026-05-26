#pragma once

#include <string>
#include "core_platform/iplatform.h"

class Config
{
    public:
    Config() = default;

    bool initConfig(const std::string &path);

    PlatformConfig getPlatformConfig() const;

    private:
    PlatformConfig platformConfig_;
};
