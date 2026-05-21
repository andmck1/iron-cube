#pragma once

#include "platform/IInput.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <cstdint>

class InputGLFW
{
    public:
    void init();

    void shutdown();

    bool isKeyPressed(KeyCode key) const;

    static void keyCallback(GLFWwindow *, int, int, int, int);

    private:
    static InputGLFW *instance_;
    std::unordered_map<int32_t, bool> key_states_;
};
