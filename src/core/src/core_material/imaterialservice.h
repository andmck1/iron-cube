#pragma once

#include "core_material/material.h"
#include <string>

class IMaterialService
{
    public:
    virtual ~IMaterialService() = default;

    virtual Material *
    registerMaterial(const std::string &name, Material material) = 0;
    virtual Material *getMaterial(const std::string &name) = 0;
    virtual std::vector<Material *> getAllMaterials() = 0;
};
