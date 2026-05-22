#pragma once

#include "render/IRenderer.h"
#include <string>

class Mesh
{
    public:
    void setGeometry(const std::string &geometryName);
    bool render();

    private:
    std::string geometryName_;
};
