#include "core_mesh/meshmanager.h"
#include <spdlog/spdlog.h>

// TODO - repeated code among the managers, this could be templated?
Mesh *MeshManager::registerMesh(const std::string &name, Mesh mesh)
{
    spdlog::debug("Registering mesh: {}", name);
    std::unique_ptr<Mesh> meshPtr = std::make_unique<Mesh>(std::move(mesh));
    Mesh *rawPtr = meshPtr.get();
    meshes_[name] = std::move(meshPtr);
    spdlog::debug("Mesh registered: {}", name);
    return rawPtr;
}

Mesh *MeshManager::getMesh(const std::string &name)
{
    auto it = meshes_.find(name);
    if (it != meshes_.end())
        return it->second.get();
    spdlog::error("Mesh not found: {}", name);
    return nullptr;
}
