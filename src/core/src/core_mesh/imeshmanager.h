#pragma once

#include "core_mesh/mesh.h"
#include <string>

class IMeshManager
{
    public:
    virtual ~IMeshManager() = default;

    virtual Mesh *registerMesh(const std::string &name, Mesh mesh) = 0;
    virtual Mesh *getMesh(const std::string &name) = 0;
};
