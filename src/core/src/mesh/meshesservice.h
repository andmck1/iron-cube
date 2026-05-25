#pragma once

#include "mesh/imeshesservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class MeshesService : public IMeshesService
{
    public:
    Mesh *registerMesh(const std::string &name, Mesh mesh) override;
    Mesh *getMesh(const std::string &name) override;

    private:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;
};
