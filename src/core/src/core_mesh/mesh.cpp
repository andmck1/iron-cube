#include "core_mesh/mesh.h"
#include "core_render/irenderer.h"
#include "core_resource/iresourcemanager.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

void Mesh::setGeometry(const std::string &geometryName)
{
    spdlog::debug("Setting mesh geometry to: {}", geometryName);
    geometryName_ = geometryName;
}

bool Mesh::render()
{
    spdlog::debug("Rendering mesh (geometry: '{}')", geometryName_);
    if (geometryName_.empty())
        return false;

    IResourceManager *resourceManager =
        ServiceLocator::instance().get<IResourceManager>();
    if (!resourceManager)
        return false;

    Geometry *geometry = resourceManager->getGeometry(geometryName_);
    if (!geometry)
        return false;

    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return false;

    renderer->drawGeometry(geometry);

    return true;
}
