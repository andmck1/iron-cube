#include "core_common/loggerservice.h"
#include <spdlog/sinks/stdout_color_sinks.h>

LoggerService::LoggerService()
{
}

void LoggerService::init(bool debugMode)
{
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%H:%M:%S %z] [%n] [%^%l%$] %v");

    logger_ = std::make_shared<spdlog::logger>("IronCube", consoleSink);
    logger_->set_level(debugMode ? spdlog::level::debug : spdlog::level::info);
    logger_->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(logger_);
}
