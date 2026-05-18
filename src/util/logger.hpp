// src/util/logger.hpp
#pragma once

#include <string>
#include <iostream>
#include <sstream>

namespace Logger
{
// Macro definitions for simplified usage in source files
#define LOGGER_INFO(msg) std::cout << "[INFO] " << msg << std::endl;
#define LOGGER_WARN(msg) std::cout << "[WARN] " << msg << std::endl;
#define LOGGER_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl;
#define LOGGER_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl;

    void
    log(const std::string &message, const char *file,
        int line); // Detailed logging function signature
} // namespace Logger