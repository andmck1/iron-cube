#pragma once

#include "core_material/imaterialservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class MaterialService : public IMaterialService
{
    public:
    Material *
    registerMaterial(const std::string &name, Material material) override;
    Material *getMaterial(const std::string &name) override;
    std::vector<Material *> getAllMaterials() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
};
