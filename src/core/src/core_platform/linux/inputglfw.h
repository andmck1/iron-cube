#pragma once

#include "core_platform/iinput.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <cstdint>

class InputGLFW : public IInput
{
    public:
    void init();
    void shutdown();
    bool isKeyPressed(KeyCode key) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    MouseDelta getMouseDelta() const override;
    void resetMouseDelta() override;
    void setWindow(GLFWwindow *window);
    void setCursorMode(bool disabled);
    static void keyCallback(GLFWwindow *, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow *, int, int, int);
    static void cursorPositionCallback(GLFWwindow *, double, double);

    private:
    static InputGLFW *instance_;
    std::unordered_map<int32_t, bool> key_states_ = {};
    std::unordered_map<int32_t, bool> mouse_button_states_ = {};
    MouseDelta mouseDelta_ = {0.0f, 0.0f};
    double lastCursorX_ = 0.0;
    double lastCursorY_ = 0.0;
    bool cursorInitialised_ = false;
    GLFWwindow *window_ = nullptr;
};
