#pragma once

#include "material/material.h"
#include <memory>
#include <string>

class IMaterialsService
{
    public:
    virtual ~IMaterialsService() = default;

    virtual Material *
    registerMaterial(const std::string &name, Material material) = 0;
    virtual Material *getMaterial(const std::string &name) = 0;
    virtual std::vector<Material *> getAllMaterials() = 0;
};
