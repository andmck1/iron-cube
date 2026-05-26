#include "core_scene/scene.h"
#include <spdlog/spdlog.h>

void Scene::addEntity(std::unique_ptr<Entity> entity)
{
    spdlog::debug("Adding entity to scene: {}", entity->getName());
    entities_.push_back(std::move(entity));
}

const std::vector<std::unique_ptr<Entity>> &Scene::getEntities()
{
    return entities_;
}
