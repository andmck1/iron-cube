#include "core_mesh/mesh.h"
#include "core_resource/iresourcemanager.h"
#include "core_service/servicelocator.h"
#include <spdlog/spdlog.h>

void Mesh::setGeometry(const std::string &geometryName)
{
    spdlog::debug("Setting mesh geometry to: {}", geometryName);
    geometryName_ = geometryName;
}

Geometry *Mesh::getGeometry()
{
    IResourceManager *resourceManager =
        ServiceLocator::instance().get<IResourceManager>();
    Geometry *geometry = resourceManager->getGeometry(geometryName_);
    return geometry;
}
