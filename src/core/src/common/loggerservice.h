#pragma once

#include "common/ilogger.h"
#include <spdlog/spdlog.h>
#include <memory>
#include <string>

class LoggerService : public ILogger
{
    public:
    LoggerService();
    ~LoggerService() override = default;

    void init(bool debugMode);
    void info(const std::string &message) override;
    void debug(const std::string &message) override;
    void warn(const std::string &message) override;
    void error(const std::string &message) override;

    private:
    std::shared_ptr<spdlog::logger> logger_;
};
