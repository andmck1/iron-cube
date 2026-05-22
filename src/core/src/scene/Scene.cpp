#include "scene/Scene.h"

void Scene::addEntity(Entity entity)
{
    entities_.push_back(entity);
}

std::vector<Entity> Scene::getEntities()
{
    return entities_;
}
