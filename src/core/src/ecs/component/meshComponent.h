// ty Vulkan!
#pragma once

#include "ecs/ecs.h"
#include "ecs/component/transformComponent.h"
#include "mesh/mesh.h"
#include "material/material.h"

class MeshComponent : public Component
{

    public:
    MeshComponent(Mesh *m, Material *mat) : mesh(m), material(mat)
    {
    }

    void setMesh(Mesh *m)
    {
        mesh = m;
    }
    void setMaterial(Material *mat)
    {
        material = mat;
    }

    Mesh *getMesh() const
    {
        return mesh;
    }
    Material *getMaterial() const
    {
        return material;
    }

    void render() override
    {
        if (!mesh || !material)
            return;

        auto transform = getOwner()->getComponent<TransformComponent>();
        if (!transform)
            return;

        material->bind();
        material->setUniform("modelMatrix", transform->getTransformMatrix());
        mesh->render();
    }

    private:
    Mesh *mesh = nullptr;
    Material *material = nullptr;
};
