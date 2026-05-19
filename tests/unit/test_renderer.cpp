#include <catch2/catch_test_macros.hpp>
#include <GLFW/glfw3.h>
#include "rendering/IRenderer.h"
#include "rendering/linux/RendererGLFW.h"

TEST_CASE("Renderer: initialize, swapBuffers, and shutdown", "[renderer]")
{
    REQUIRE(glfwInit());

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    REQUIRE(window != nullptr);

    RendererGLFW renderer;
    REQUIRE(renderer.init(window));

    renderer.swapBuffers();

    renderer.shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();
}
