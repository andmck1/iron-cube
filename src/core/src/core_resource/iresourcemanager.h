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

    virtual void loadTexture(const std::string &name, Texture texture) = 0;
    virtual Texture *getTexture(const std::string &name) = 0;
    virtual void unloadTexture(const std::string &name) = 0;
};
