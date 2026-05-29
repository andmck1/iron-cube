#pragma once

#include <string>

class Mesh
{
    public:
    void setGeometry(const std::string &geometryName);
    bool render();

    protected:
    std::string geometryName_;
};
