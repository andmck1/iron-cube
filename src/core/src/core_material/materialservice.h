#pragma once

#include "core_material/imaterialservice.h"
#include <memory>
#include <string>
#include <unordered_map>

class MaterialService : public IMaterialService
{
    public:
    IMaterial *registerMaterial(
        const std::string &name, std::unique_ptr<IMaterial> material
    ) override;
    IMaterial *getMaterial(const std::string &name) override;
    std::vector<IMaterial *> getAllMaterials() override;

    private:
    std::unordered_map<std::string, std::unique_ptr<IMaterial>> materials_;
};
