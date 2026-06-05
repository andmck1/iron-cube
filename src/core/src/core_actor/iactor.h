#pragma once

#include "core_ecs/ecs.h"
#include "core_ecs/component/transform.h"

class IActor : public Entity
{
    public:
    explicit IActor(const std::string &entityName)
        : Entity(entityName)
    {
        addComponent<TransformComponent>();
    }

    virtual ~IActor() = default;
};
