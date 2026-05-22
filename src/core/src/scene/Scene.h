#pragma once

#include "ecs/ecs.h"

class Scene
{
    public:
    void addEntity(Entity entity);
    std::vector<Entity> getEntities();

    private:
    std::vector<Entity> entities_;
};
