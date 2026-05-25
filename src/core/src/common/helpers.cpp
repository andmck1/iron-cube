#include "common/helpers.h"

#include <iostream>

void printUsage(const std::string &progName)
{
    std::cerr << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  --debug    Enable debug-level logging\n"
              << "  --help     Show this help message\n";
}
