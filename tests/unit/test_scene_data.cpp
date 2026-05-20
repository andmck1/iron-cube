#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "scene/SceneData.h"
#include "scene/IScene.h"
#include <type_traits>
#include <cstdint>

TEST_CASE("Vertex struct has correct memory layout", "[scene][vertex]")
{
    STATIC_REQUIRE(std::is_standard_layout_v<Vertex>);
    STATIC_REQUIRE(sizeof(Vertex::pos) == sizeof(glm::vec2));
    STATIC_REQUIRE(sizeof(glm::vec2) + sizeof(glm::vec3) == sizeof(Vertex));
}

TEST_CASE("SceneData default construction yields empty state", "[scene][scenedata]")
{
    SceneData data;
    CHECK(data.vertices.empty());
    CHECK(data.vertex_count == 0);
}

TEST_CASE("SceneData vertex insertion updates count", "[scene][scenedata]")
{
    SceneData data;
    data.vertices.push_back(Vertex{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}});
    data.vertices.push_back(Vertex{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}});
    data.vertices.push_back(Vertex{{0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}});
    data.vertex_count = static_cast<uint32_t>(data.vertices.size());

    CHECK(data.vertices.size() == 3);
    CHECK(data.vertex_count == 3);
}

TEST_CASE("SceneData validation rejects vertex count mismatches", "[scene][scenedata]")
{
    SceneData data;
    data.vertices.push_back(Vertex{{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    data.vertex_count = 5;

    CHECK(!data.isValid());
}

TEST_CASE("SceneData validation accepts matching counts", "[scene][scenedata]")
{
    SceneData data;
    data.vertices.push_back(Vertex{{0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    data.vertex_count = 1;

    CHECK(data.isValid());
}

TEST_CASE("IScene interface has virtual destructor", "[scene][interface]")
{
    STATIC_REQUIRE(std::has_virtual_destructor_v<IScene>);
}

TEST_CASE("IScene isDirty returns true after initialize", "[scene][interface]")
{
    struct TestScene : public IScene {
        bool initialize() override { dirty_ = true; return true; }
        const std::vector<Vertex>& getVertices() const override { return vertices_; }
        uint32_t getVertexCount() const override { return static_cast<uint32_t>(vertices_.size()); }
        void shutdown() override { vertices_.clear(); dirty_ = false; }
        bool isDirty() const override { return dirty_; }
        void markClean() override { dirty_ = false; }

        std::vector<Vertex> vertices_;
        bool dirty_ = false;
    };

    TestScene scene;
    CHECK(!scene.isDirty());
    scene.initialize();
    CHECK(scene.isDirty());
}

TEST_CASE("IScene markClean resets dirty flag", "[scene][interface]")
{
    struct TestScene : public IScene {
        bool initialize() override { dirty_ = true; return true; }
        const std::vector<Vertex>& getVertices() const override { return vertices_; }
        uint32_t getVertexCount() const override { return static_cast<uint32_t>(vertices_.size()); }
        void shutdown() override { vertices_.clear(); dirty_ = false; }
        bool isDirty() const override { return dirty_; }
        void markClean() override { dirty_ = false; }

        std::vector<Vertex> vertices_;
        bool dirty_ = false;
    };

    TestScene scene;
    scene.initialize();
    CHECK(scene.isDirty());
    scene.markClean();
    CHECK(!scene.isDirty());
}

TEST_CASE("IScene polymorphic deletion safety", "[scene][interface]")
{
    struct TestScene : public IScene {
        bool deleted = false;
        ~TestScene() override { deleted = true; }
        bool initialize() override { return true; }
        const std::vector<Vertex>& getVertices() const override { static std::vector<Vertex> v; return v; }
        uint32_t getVertexCount() const override { return 0; }
        void shutdown() override {}
        bool isDirty() const override { return false; }
        void markClean() override {}
    };

    IScene* scene = new TestScene();
    bool* deletedPtr = &static_cast<TestScene*>(scene)->deleted;
    delete scene;
    CHECK(*deletedPtr);
}
