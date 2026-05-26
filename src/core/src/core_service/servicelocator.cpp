#include "core_service/servicelocator.h"

ServiceLocator &ServiceLocator::instance()
{
    static ServiceLocator loc;
    return loc;
}
