#pragma once

#include "material/imaterialsservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class MaterialsService : public IMaterialsService
{
    public:
    Material *
    registerMaterial(const std::string &name, Material material) override;
    Material *getMaterial(const std::string &name) override;
    std::vector<Material *> getAllMaterials() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
};
