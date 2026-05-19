#include "service/ServiceLocator.h"

ServiceLocator &ServiceLocator::instance()
{
    static ServiceLocator loc;
    return loc;
}
