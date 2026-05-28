#pragma once

#include "core_mesh/imeshservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class MeshService : public IMeshService
{
    public:
    Mesh *registerMesh(const std::string &name, Mesh mesh) override;
    Mesh *getMesh(const std::string &name) override;

    private:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;
};
