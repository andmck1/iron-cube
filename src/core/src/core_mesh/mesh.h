#pragma once

#include "core_common/types.h"
#include <string>

class Mesh
{
    public:
    void setGeometry(const std::string &geometryName);
    Geometry *getGeometry();

    protected:
    std::string geometryName_;
};
