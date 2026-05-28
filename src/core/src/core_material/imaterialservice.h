#pragma once

#include "core_material/imaterial.h"
#include <memory>
#include <vector>
#include <string>

class IMaterialService
{
    public:
    virtual ~IMaterialService() = default;

    virtual IMaterial *registerMaterial(
        const std::string &name, std::unique_ptr<IMaterial> material
    ) = 0;
    virtual IMaterial *getMaterial(const std::string &name) = 0;
    virtual std::vector<IMaterial *> getAllMaterials() = 0;
};
