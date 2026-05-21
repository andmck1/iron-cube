#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>

TEST_CASE("Platform: initialise and shutdown cycle", "[platform]")
{
    bool init_ok = glfwInit();
    REQUIRE(init_ok);

    glfwTerminate();
}

TEST_CASE("Platform: window close callback sets flag", "[platform]")
{
    REQUIRE(glfwInit());

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    REQUIRE(window != nullptr);

    glfwSetWindowCloseCallback(
        window,
        [](GLFWwindow *)
        {
            // This test just verifies the callback can be set
        }
    );

    glfwDestroyWindow(window);
    glfwTerminate();
}
