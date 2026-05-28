// ty Vulkan!
#pragma once

#include "core_ecs/ecs.h"
#include "core_mesh/mesh.h"
#include "core_material/imaterial.h"

class MeshComponent : public Component
{

    public:
    void setMesh(Mesh *m);
    void setMaterial(IMaterial *mat);

    Mesh *getMesh() const;
    IMaterial *getMaterial() const;

    void render();

    private:
    Mesh *mesh = nullptr;
    IMaterial *material = nullptr;
};
