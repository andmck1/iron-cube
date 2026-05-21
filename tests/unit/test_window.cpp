#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>

TEST_CASE("Window creation: GLFW initialises successfully", "[window]")
{
    REQUIRE(glfwInit());
    glfwTerminate();
}

TEST_CASE("Window creation: window can be created and destroyed", "[window]")
{
    REQUIRE(glfwInit());

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    REQUIRE(window != nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}
