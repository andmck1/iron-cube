#pragma once

#include "core_actor/iactor.h"
#include "core_ecs/component/mesh.h"

class SceneActor : public IActor
{
    public:
    explicit SceneActor(const std::string &entityName) : IActor(entityName)
    {
        addComponent<MeshComponent>();
    }

    ~SceneActor() = default;
};
