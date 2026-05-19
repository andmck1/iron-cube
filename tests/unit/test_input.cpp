#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>

TEST_CASE("Input: key state tracking works", "[input]")
{
    REQUIRE(glfwInit());

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    REQUIRE(window != nullptr);

    // Key not pressed initially
    REQUIRE(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);

    glfwDestroyWindow(window);
    glfwTerminate();
}
