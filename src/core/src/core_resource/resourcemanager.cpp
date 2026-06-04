#include "core_resource/resourcemanager.h"
#include <spdlog/spdlog.h>

bool ResourceManager::loadGeometry(const std::string &name, Geometry geometry)
{
    spdlog::debug("Loading geometry '{}'", name);

    geometries_[name] = geometry;
    spdlog::info(
        "Loaded geometry '{}': {} vertices", name, geometry.vertexCount
    );
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

Geometry ResourceManager::generateProceduralTriangle()
{
    Geometry geometry;
    geometry.vertices.reserve(3);
    geometry.indices.reserve(3);

    geometry.vertices.push_back(
        {glm::vec3(0.f, -0.5f, 0.f), glm::vec2(0.5f, 0.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, 0.f), glm::vec2(0.f, 1.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, 0.f), glm::vec2(1.f, 1.f),
         glm::vec3(0.f, 0.f, -1.f)}
    );

    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);

    geometry.vertexCount = static_cast<uint32_t>(geometry.vertices.size());

    spdlog::info("Generated procedural triangle geometry");
    return geometry;
}

Geometry ResourceManager::generateProceduralRectangle()
{
    Geometry geometry;
    geometry.vertices.reserve(4);
    geometry.indices.reserve(6);

    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, 0.f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, 0.f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, 0.f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.f, 0.f, -1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, 0.f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.f, 0.f, -1.f)}
    );

    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);
    geometry.indices.push_back(2);
    geometry.indices.push_back(3);
    geometry.indices.push_back(0);

    geometry.vertexCount = static_cast<uint32_t>(geometry.vertices.size());

    spdlog::info("Generated procedural rectangle geometry");
    return geometry;
}

Geometry ResourceManager::generateProceduralCube()
{
    Geometry geometry;
    geometry.vertices.reserve(24);
    geometry.indices.reserve(36);

    // Front face (z = +0.5) — vertices 0-3
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)}
    );
    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);
    geometry.indices.push_back(0);
    geometry.indices.push_back(2);
    geometry.indices.push_back(3);

    // Back face (z = -0.5) — vertices 4-7
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 0.0f, -1.0f)}
    );
    geometry.indices.push_back(4);
    geometry.indices.push_back(5);
    geometry.indices.push_back(6);
    geometry.indices.push_back(4);
    geometry.indices.push_back(6);
    geometry.indices.push_back(7);

    // Top face (y = +0.5) — vertices 8-11
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, 1.0f, 0.0f)}
    );
    geometry.indices.push_back(8);
    geometry.indices.push_back(9);
    geometry.indices.push_back(10);
    geometry.indices.push_back(8);
    geometry.indices.push_back(10);
    geometry.indices.push_back(11);

    // Bottom face (y = -0.5) — vertices 12-15
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(0.0f, -1.0f, 0.0f)}
    );
    geometry.indices.push_back(12);
    geometry.indices.push_back(13);
    geometry.indices.push_back(14);
    geometry.indices.push_back(12);
    geometry.indices.push_back(14);
    geometry.indices.push_back(15);

    // Right face (x = +0.5) — vertices 16-19
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(1.0f, 0.0f, 0.0f)}
    );
    geometry.indices.push_back(16);
    geometry.indices.push_back(17);
    geometry.indices.push_back(18);
    geometry.indices.push_back(16);
    geometry.indices.push_back(18);
    geometry.indices.push_back(19);

    // Left face (x = -0.5) — vertices 20-23
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0.0f, 1.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1.0f, 1.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    geometry.vertices.push_back(
        {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1.0f, 0.0f),
         glm::vec3(-1.0f, 0.0f, 0.0f)}
    );
    geometry.indices.push_back(20);
    geometry.indices.push_back(21);
    geometry.indices.push_back(22);
    geometry.indices.push_back(20);
    geometry.indices.push_back(22);
    geometry.indices.push_back(23);

    geometry.vertexCount = static_cast<uint32_t>(geometry.vertices.size());

    spdlog::info("Generated procedural cube geometry");
    return geometry;
}
