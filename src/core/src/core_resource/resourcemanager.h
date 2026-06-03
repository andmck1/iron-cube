#pragma once

#include "core_resource/iresourcemanager.h"
#include <spdlog/spdlog.h>

class ResourceManager : public IResourceManager
{
    public:
    bool loadGeometry(
        const std::string &name,
        std::shared_ptr<std::vector<VertexInputType>> verts,
        uint32_t vertexCount
    ) override;
    Geometry *getGeometry(const std::string &name) override;
    bool unloadGeometry(const std::string &name) override;

    static std::shared_ptr<std::vector<VertexInputType>>
    generateProceduralTriangle();
    static std::shared_ptr<std::vector<VertexInputType>>
    generateProceduralCube();

    private:
    std::unordered_map<std::string, Geometry> geometries_;
};
