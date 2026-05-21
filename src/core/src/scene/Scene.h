#pragma once

#include "ecs/ecs.h"

class Scene
{
    public:
    void addEntity(Entity entity);

    private:
    std::vector<Entity> entities;
};
