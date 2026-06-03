#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

ServiceLocator &ServiceLocator::instance()
{
    static ServiceLocator loc;
    return loc;
}
