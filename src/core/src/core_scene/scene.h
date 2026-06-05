#pragma once

#include "core_ecs/ecs.h"
#include <vector>

class Scene
{
    public:
    void addEntity(std::unique_ptr<Entity> entity);
    const std::vector<std::unique_ptr<Entity>> &getEntities();

    private:
    std::vector<std::unique_ptr<Entity>> entities_;
};
