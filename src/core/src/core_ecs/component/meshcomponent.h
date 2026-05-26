// ty Vulkan!
#pragma once

#include "core_ecs/ecs.h"
#include "core_mesh/mesh.h"
#include "core_material/material.h"

class MeshComponent : public Component
{

    public:
    void setMesh(Mesh *m);
    void setMaterial(Material *mat);

    Mesh *getMesh() const;
    Material *getMaterial() const;

    void render();

    private:
    Mesh *mesh = nullptr;
    Material *material = nullptr;
};
