// ty Vulkan!
#pragma once

#include "core_ecs/ecs.h"
#include "core_mesh/mesh.h"
#include "core_render/ishaderpipeline.h"

class MeshComponent : public Component
{

    public:
    void setMesh(Mesh *mesh);
    void setShaderPipeline(IShaderPipeline *shaderPipeline);
    void render();

    Mesh *getMesh() const;
    IShaderPipeline *getShaderPipeline() const;

    private:
    Mesh *mesh_ = nullptr;
    IShaderPipeline *shaderPipeline_ = nullptr;
};
