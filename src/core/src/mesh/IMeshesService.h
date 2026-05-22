#pragma once

#include "mesh/mesh.h"
#include <memory>
#include <string>

class IMeshesService
{
    public:
    virtual ~IMeshesService() = default;

    virtual Mesh *registerMesh(const std::string &name, Mesh mesh) = 0;
    virtual Mesh *getMesh(const std::string &name) = 0;
};
