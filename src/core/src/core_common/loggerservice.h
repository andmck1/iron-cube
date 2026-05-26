#pragma once

#include "core_common/ilogger.h"
#include <spdlog/spdlog.h>
#include <memory>

class LoggerService : public ILogger
{
    public:
    LoggerService();
    ~LoggerService() override = default;

    void init(bool debugMode);

    private:
    std::shared_ptr<spdlog::logger> logger_;
};
