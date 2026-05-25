// ty Vulkan!
#pragma once

#include "ecs/ecs.h"
#include "mesh/mesh.h"
#include "material/material.h"

class MeshComponent : public Component
{

    public:
    MeshComponent(Mesh *m, Material *mat);

    void setMesh(Mesh *m);
    void setMaterial(Material *mat);

    Mesh *getMesh() const;
    Material *getMaterial() const;

    void render();

    private:
    Mesh *mesh = nullptr;
    Material *material = nullptr;
};
