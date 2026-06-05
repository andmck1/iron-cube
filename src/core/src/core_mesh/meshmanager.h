#pragma once

#include "core_mesh/imeshmanager.h"
#include <memory>
#include <string>
#include <unordered_map>

class MeshManager : public IMeshManager
{
    public:
    Mesh *registerMesh(const std::string &name, Mesh mesh) override;
    Mesh *getMesh(const std::string &name) override;

    protected:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;
};
