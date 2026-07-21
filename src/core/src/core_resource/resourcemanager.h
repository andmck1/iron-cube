#pragma once

#include "core_resource/iresourcemanager.h"
#include <spdlog/spdlog.h>
#include <string>

class ResourceManager : public IResourceManager
{
    public:
    bool loadGeometry(const std::string &name, Geometry geometry) override;
    Geometry *getGeometry(const std::string &name) override;
    bool unloadGeometry(const std::string &name) override;

    void loadTexture(const std::string &name, Texture texture) override;
    Texture *getTexture(const std::string &name) override;
    void unloadTexture(const std::string &name) override;

    static Geometry generateProceduralTriangle();
    static Geometry generateProceduralRectangle();
    static Geometry generateProceduralCube();
    static Geometry
    loadObj(std::vector<std::string> objFilePaths, std::string objBaseDir);
    static Texture loadImage(std::string filepath);

    private:
    static void processMeshlets(Geometry *geometry);
    static void repackMeshlets(Geometry *geometry);
    static void calculateMeshBounds(Geometry *geometry);

    std::unordered_map<std::string, Geometry> geometries_;
    std::unordered_map<std::string, Texture> textures_;
};
