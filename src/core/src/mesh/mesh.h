#pragma once

#include "render/irenderer.h"
#include <string>

class Mesh
{
    public:
    void setGeometry(const std::string &geometryName);
    bool render();

    private:
    std::string geometryName_;
};
