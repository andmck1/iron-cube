#include "core_material/materialservice.h"
#include <spdlog/spdlog.h>

Material *
MaterialService::registerMaterial(const std::string &name, Material material)
{
    spdlog::debug("Registering material: {}", name);
    std::unique_ptr<Material> materialPtr =
        std::make_unique<Material>(std::move(material));
    Material *rawPtr = materialPtr.get();
    materials_[name] = std::move(materialPtr);
    spdlog::debug("Material registered: {}", name);
    return rawPtr;
}

Material *MaterialService::getMaterial(const std::string &name)
{
    auto it = materials_.find(name);
    if (it != materials_.end())
        return it->second.get();
    spdlog::error("Material not found: {}", name);
    return nullptr;
}

std::vector<Material *> MaterialService::getAllMaterials()
{
    std::vector<Material *> result;
    for (auto &pair : materials_)
    {
        result.push_back(pair.second.get());
    }
    return result;
}
