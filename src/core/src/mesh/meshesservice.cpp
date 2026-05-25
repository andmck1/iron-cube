#include "mesh/meshesservice.h"
#include <spdlog/spdlog.h>

Mesh *MeshesService::registerMesh(const std::string &name, Mesh mesh)
{
    spdlog::debug("Registering mesh: {}", name);
    auto meshPtr = std::make_unique<Mesh>(std::move(mesh));
    Mesh *rawPtr = meshPtr.get();
    meshes_[name] = std::move(meshPtr);
    spdlog::debug("Mesh registered: {}", name);
    return rawPtr;
}

Mesh *MeshesService::getMesh(const std::string &name)
{
    auto it = meshes_.find(name);
    if (it != meshes_.end())
        return it->second.get();
    spdlog::error("Mesh not found: {}", name);
    return nullptr;
}
