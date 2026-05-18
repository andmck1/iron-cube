// src/util/logger.cpp
#include "logger.hpp"

void Logger::log(const std::string &message, const char *file, int line)
{
    std::stringstream ss;
    ss << "[LOG] [" << file << ":" << line << "] " << message;
    std::cout << ss.str() << std::endl;
}
