#include "mesh/mesh.h"
#include "render/irenderer.h"
#include "resource/iresourcemanager.h"
#include "service/servicelocator.h"

void Mesh::setGeometry(const std::string &geometryName)
{
    geometryName_ = geometryName;
}

bool Mesh::render()
{
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
