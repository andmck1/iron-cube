#pragma once

#include "core_common/types.h"
#include <string>

class IResourceManager
{
    public:
    virtual ~IResourceManager() = default;

    virtual bool loadGeometry(const std::string &name, Geometry geometry) = 0;

    virtual Geometry *getGeometry(const std::string &name) = 0;
    virtual bool unloadGeometry(const std::string &name) = 0;
};
