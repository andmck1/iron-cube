#pragma once

#include "core_common/types.h"
#include <string>

class IResourceManager
{
    public:
    virtual ~IResourceManager() = default;

    virtual bool loadGeometry(
        const std::string &name,
        std::shared_ptr<std::vector<VertexInputType>> verts,
        uint32_t vertexCount
    ) = 0;

    virtual Geometry *getGeometry(const std::string &name) = 0;
    virtual bool unloadGeometry(const std::string &name) = 0;
};
