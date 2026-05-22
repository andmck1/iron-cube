#include "material/MaterialsService.h"
#include <spdlog/spdlog.h>

Material *
MaterialsService::registerMaterial(const std::string &name, Material material)
{
    auto materialPtr = std::make_unique<Material>(std::move(material));
    Material *rawPtr = materialPtr.get();
    materials_[name] = std::move(materialPtr);
    return rawPtr;
}

Material *MaterialsService::getMaterial(const std::string &name)
{
    auto it = materials_.find(name);
    if (it != materials_.end())
        return it->second.get();
    spdlog::error("Material not found: {}", name);
    return nullptr;
}
