#include "scene/Scene.h"

void Scene::addEntity(std::unique_ptr<Entity> entity)
{
    entities_.push_back(std::move(entity));
}

const std::vector<std::unique_ptr<Entity>> &Scene::getEntities()
{
    return entities_;
}
