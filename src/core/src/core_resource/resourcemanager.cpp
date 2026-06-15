#include "core_resource/resourcemanager.h"
#include "meshoptimizer.h"
#include <tiny_obj_loader.h>
#include <spdlog/spdlog.h>

bool ResourceManager::loadGeometry(const std::string &name, Geometry geometry)
{
    spdlog::debug("Loading geometry '{}'", name);

    geometries_[name] = geometry;
    spdlog::info(
        "Loaded geometry '{}': {} vertices", name, geometry.indices.size()
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
        {glm::vec4(0.f, -0.5f, 0.f, 1.f), glm::vec4(0.5f, 0.f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, 0.f, 1.f), glm::vec4(1.f, 1.f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );

    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);

    processMeshlets(&geometry);
    calculateMeshBounds(&geometry);

    spdlog::info("Generated procedural triangle geometry");
    return geometry;
}

Geometry ResourceManager::generateProceduralRectangle()
{
    Geometry geometry;
    geometry.vertices.reserve(4);
    geometry.indices.reserve(6);

    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, 0.f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, 0.f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, 0.f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, 0.f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.f, 0.f, -1.f, 1.f)}
    );

    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);
    geometry.indices.push_back(2);
    geometry.indices.push_back(3);
    geometry.indices.push_back(0);

    processMeshlets(&geometry);
    calculateMeshBounds(&geometry);

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
        {glm::vec4(-0.5f, -0.5f, 0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, 1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, 0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, 1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, 1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, 0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, 1.0f, 1.f)}
    );
    geometry.indices.push_back(0);
    geometry.indices.push_back(1);
    geometry.indices.push_back(2);
    geometry.indices.push_back(0);
    geometry.indices.push_back(2);
    geometry.indices.push_back(3);

    // Back face (z = -0.5) — vertices 4-7
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, -0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, -1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, -0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, -1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, -0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, -1.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, -0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 0.0f, -1.0f, 1.f)}
    );
    geometry.indices.push_back(4);
    geometry.indices.push_back(5);
    geometry.indices.push_back(6);
    geometry.indices.push_back(4);
    geometry.indices.push_back(6);
    geometry.indices.push_back(7);

    // Top face (y = +0.5) — vertices 8-11
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, -0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, 0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, 1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, -0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, 1.0f, 0.0f, 1.f)}
    );
    geometry.indices.push_back(8);
    geometry.indices.push_back(9);
    geometry.indices.push_back(10);
    geometry.indices.push_back(8);
    geometry.indices.push_back(10);
    geometry.indices.push_back(11);

    // Bottom face (y = -0.5) — vertices 12-15
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, -0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, -1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, -0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(0.0f, -1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, 0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, -1.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, 0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(0.0f, -1.0f, 0.0f, 1.f)}
    );
    geometry.indices.push_back(12);
    geometry.indices.push_back(13);
    geometry.indices.push_back(14);
    geometry.indices.push_back(12);
    geometry.indices.push_back(14);
    geometry.indices.push_back(15);

    // Right face (x = +0.5) — vertices 16-19
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, 0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, 0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, 0.5f, -0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(0.5f, -0.5f, -0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.indices.push_back(16);
    geometry.indices.push_back(18);
    geometry.indices.push_back(17);
    geometry.indices.push_back(16);
    geometry.indices.push_back(19);
    geometry.indices.push_back(18);

    // Left face (x = -0.5) — vertices 20-23
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, -0.5f, 1.f), glm::vec4(0.0f, 0.0f, 0.f, 0.f),
         glm::vec4(-1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, -0.5f, 1.f), glm::vec4(0.0f, 1.0f, 0.f, 0.f),
         glm::vec4(-1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, 0.5f, 0.5f, 1.f), glm::vec4(1.0f, 1.0f, 0.f, 0.f),
         glm::vec4(-1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.vertices.push_back(
        {glm::vec4(-0.5f, -0.5f, 0.5f, 1.f), glm::vec4(1.0f, 0.0f, 0.f, 0.f),
         glm::vec4(-1.0f, 0.0f, 0.0f, 1.f)}
    );
    geometry.indices.push_back(20);
    geometry.indices.push_back(22);
    geometry.indices.push_back(21);
    geometry.indices.push_back(20);
    geometry.indices.push_back(23);
    geometry.indices.push_back(22);

    processMeshlets(&geometry);
    calculateMeshBounds(&geometry);

    spdlog::info("Generated procedural cube geometry");
    return geometry;
}

// AI helped with this
// Really useful example where this has just worked
Geometry
ResourceManager::loadObj(std::string objFilePath, std::string objBaseDir)
{
    Geometry geometry;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;
    bool res = tinyobj::LoadObj(
        &attrib, &shapes, &materials, &warn, &err, objFilePath.c_str(),
        objBaseDir.c_str()
    );

    if (!warn.empty())
    {
        spdlog::warn("OBJ load warning for '{}': {}", objFilePath, warn);
    }
    if (!err.empty())
    {
        spdlog::error("OBJ load error for '{}': {}", objFilePath, err);
    }
    if (!res)
    {
        spdlog::error("Failed to load OBJ file: {}", objFilePath);
        return geometry;
    }

    for (const auto &shape : shapes)
    {
        for (const auto &idx : shape.mesh.indices)
        {
            // Position
            glm::vec3 position(0.f, 0.f, 0.f);
            if (idx.vertex_index >= 0 &&
                static_cast<size_t>(idx.vertex_index * 3 + 2) <
                    attrib.vertices.size())
            {
                position = glm::vec3(
                    attrib.vertices[idx.vertex_index * 3 + 0],
                    attrib.vertices[idx.vertex_index * 3 + 1],
                    attrib.vertices[idx.vertex_index * 3 + 2]
                );
            }

            // Normal
            glm::vec3 normal(0.f, 0.f, -1.f);
            if (idx.normal_index >= 0 &&
                static_cast<size_t>(idx.normal_index * 3 + 2) <
                    attrib.normals.size())
            {
                normal = glm::vec3(
                    attrib.normals[idx.normal_index * 3 + 0],
                    attrib.normals[idx.normal_index * 3 + 1],
                    attrib.normals[idx.normal_index * 3 + 2]
                );
            }

            // Tex coord
            glm::vec2 tex(0.f, 0.f);
            if (idx.texcoord_index >= 0 &&
                static_cast<size_t>(idx.texcoord_index * 2 + 1) <
                    attrib.texcoords.size())
            {
                tex = glm::vec2(
                    attrib.texcoords[idx.texcoord_index * 2 + 0],
                    attrib.texcoords[idx.texcoord_index * 2 + 1]
                );
            }

            geometry.vertices.push_back(
                {glm::vec4(position.x, position.y, position.z, 1.f),
                 glm::vec4(tex.x, tex.y, 0.f, 0.f),
                 glm::vec4(normal.x, normal.y, normal.z, 1.f)}
            );

            geometry.indices.push_back(
                static_cast<uint16_t>(geometry.vertices.size() - 1)
            );
        }
    }

    processMeshlets(&geometry);
    calculateMeshBounds(&geometry);

    spdlog::info(
        "Loaded OBJ '{}': {} vertices, {} indices, {} meshlets", objFilePath,
        geometry.vertices.size(), geometry.indices.size(),
        geometry.meshlets.size()
    );
    return geometry;
}

// Help from chaoticbob tutorial
void ResourceManager::processMeshlets(Geometry *geometry)
{
    std::vector<glm::vec4> positions;

    for (const VertexInputType &vertex : geometry->vertices)
    {
        positions.push_back(vertex.position);
    }

    const size_t kMaxVertices = 64;
    const size_t kMaxTriangles = 124;
    const float kConeWeight = 0.0f;

    const size_t maxMeshlets = meshopt_buildMeshletsBound(
        geometry->indices.size(), kMaxVertices, kMaxTriangles
    );

    geometry->meshlets.resize(maxMeshlets);
    geometry->meshletVertices.resize(maxMeshlets * kMaxVertices);
    geometry->meshletTriangles.resize(maxMeshlets * kMaxTriangles * 3);

    // TODO - this feels like a hack, should make indices 32 bit generally?
    // meshopt requires uint32_t indices, so convert from uint16_t
    std::vector<uint32_t> indices32(
        geometry->indices.begin(), geometry->indices.end()
    );

    size_t meshletCount = meshopt_buildMeshlets(
        geometry->meshlets.data(), geometry->meshletVertices.data(),
        geometry->meshletTriangles.data(), indices32.data(), indices32.size(),
        reinterpret_cast<const float *>(positions.data()), positions.size(),
        sizeof(glm::vec4), kMaxVertices, kMaxTriangles, kConeWeight
    );

    auto &last = geometry->meshlets[meshletCount - 1];
    geometry->meshletVertices.resize(last.vertex_offset + last.vertex_count);
    geometry->meshletTriangles.resize(
        last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3)
    );
    geometry->meshlets.resize(meshletCount);

    // repack for gpu
    for (auto &m : geometry->meshlets)
    {
        // Save triangle offset for current meshlet
        uint32_t triangleOffset =
            static_cast<uint32_t>(geometry->meshletTrianglesU32.size());

        // Repack to uint32_t
        for (uint32_t i = 0; i < m.triangle_count; ++i)
        {
            uint32_t i0 = 3 * i + 0 + m.triangle_offset;
            uint32_t i1 = 3 * i + 1 + m.triangle_offset;
            uint32_t i2 = 3 * i + 2 + m.triangle_offset;

            uint8_t vIdx0 = geometry->meshletTriangles[i0];
            uint8_t vIdx1 = geometry->meshletTriangles[i1];
            uint8_t vIdx2 = geometry->meshletTriangles[i2];
            uint32_t packed = ((static_cast<uint32_t>(vIdx0) & 0xFF) << 0) |
                              ((static_cast<uint32_t>(vIdx1) & 0xFF) << 8) |
                              ((static_cast<uint32_t>(vIdx2) & 0xFF) << 16);
            geometry->meshletTrianglesU32.push_back(packed);
        }

        // Update triangle offset for current meshlet
        m.triangle_offset = triangleOffset;
    }
}

void ResourceManager::calculateMeshBounds(Geometry *geometry)
{
    if (geometry->vertices.empty())
    {
        geometry->meshBounds = {{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}};
        return;
    }

    glm::vec3 minCorner = glm::vec3(
        geometry->vertices[0].position.x, geometry->vertices[0].position.y,
        geometry->vertices[0].position.z
    );
    glm::vec3 maxCorner = glm::vec3(
        geometry->vertices[0].position.x, geometry->vertices[0].position.y,
        geometry->vertices[0].position.z
    );

    for (size_t i = 1; i < geometry->vertices.size(); ++i)
    {
        glm::vec3 pos = glm::vec3(
            geometry->vertices[i].position.x, geometry->vertices[i].position.y,
            geometry->vertices[i].position.z
        );
        minCorner = glm::min(minCorner, pos);
        maxCorner = glm::max(maxCorner, pos);
    }

    geometry->meshBounds = {minCorner, maxCorner};
}
