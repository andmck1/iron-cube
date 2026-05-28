#include "core_material/materialservice.h"
#include <spdlog/spdlog.h>

IMaterial *MaterialService::registerMaterial(
    const std::string &name, std::unique_ptr<IMaterial> material
)
{
    spdlog::debug("Registering material: {}", name);
    IMaterial *rawPtr = material.get();
    materials_[name] = std::move(material);
    spdlog::debug("Material registered: {}", name);
    return rawPtr;
}

IMaterial *MaterialService::getMaterial(const std::string &name)
{
    auto it = materials_.find(name);
    if (it != materials_.end())
        return it->second.get();
    spdlog::error("Material not found: {}", name);
    return nullptr;
}

std::vector<IMaterial *> MaterialService::getAllMaterials()
{
    std::vector<IMaterial *> result;
    for (auto &pair : materials_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}
