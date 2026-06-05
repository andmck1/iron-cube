#pragma once

#include "core_resource/iresourcemanager.h"
#include <spdlog/spdlog.h>

class ResourceManager : public IResourceManager
{
    public:
    bool loadGeometry(const std::string &name, Geometry geometry) override;
    Geometry *getGeometry(const std::string &name) override;
    bool unloadGeometry(const std::string &name) override;

    static Geometry generateProceduralTriangle();
    static Geometry generateProceduralRectangle();
    static Geometry generateProceduralCube();

    private:
    std::unordered_map<std::string, Geometry> geometries_;
};
