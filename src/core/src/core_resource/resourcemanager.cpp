#include "core_resource/resourcemanager.h"
#include <spdlog/spdlog.h>

bool ResourceManager::loadGeometry(
    const std::string &name,
    std::shared_ptr<std::vector<VertexInputType>> verts, uint32_t vertexCount
)
{
    spdlog::debug("Loading geometry '{}': {} vertices", name, vertexCount);
    if (!verts || vertexCount == 0)
    {
        spdlog::error("Failed to load geometry '{}': invalid data", name);
        return false;
    }

    Geometry geom{};
    geom.vertices = verts;
    geom.vertexCount = vertexCount;
    geometries_[name] = geom;
    spdlog::info("Loaded geometry '{}': {} vertices", name, vertexCount);
    return true;
}

Geometry *ResourceManager::getGeometry(const std::string &name)
{
    spdlog::debug("Requesting geometry: {}", name);
    auto it = geometries_.find(name);
    if (it != geometries_.end())
    {
        spdlog::debug("Found geometry: {}", name);
        return &(it->second);
    }
    spdlog::error("Geometry not found: {}", name);
    return nullptr;
}

bool ResourceManager::unloadGeometry(const std::string &name)
{
    auto it = geometries_.find(name);
    if (it != geometries_.end())
    {
        geometries_.erase(it);
        spdlog::info("Unloaded geometry '{}'", name);
        return true;
    }
    spdlog::error("Cannot unload geometry '{}': not found", name);
    return false;
}

std::shared_ptr<std::vector<VertexInputType>>
ResourceManager::generateProceduralTriangle()
{
    auto vertices = std::make_shared<std::vector<VertexInputType>>();
    vertices->reserve(3);

    vertices->push_back(
        {glm::vec3(0.f, 1.f, 0.f), glm::vec2(0.5f, 0.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    vertices->push_back(
        {glm::vec3(-1.f, 0.f, 0.f), glm::vec2(0.f, 1.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    vertices->push_back(
        {glm::vec3(1.f, 0.f, 0.f), glm::vec2(1.f, 1.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );

    spdlog::info("Generated procedural triangle geometry");
    return vertices;
}

std::shared_ptr<std::vector<VertexInputType>>
ResourceManager::generateProceduralCube()
{
    auto vertices = std::make_shared<std::vector<VertexInputType>>();
    vertices->reserve(36);

    // Front face (z = +0.5)
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );

    // Back face (z = -0.5)
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );

    // Top face (y = +0.5)
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );

    // Bottom face (y = -0.5)
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );

    // Right face (x = +0.5)
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );

    // Left face (x = -0.5)
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    vertices->push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );

    spdlog::info("Generated procedural cube geometry");
    return vertices;
}
