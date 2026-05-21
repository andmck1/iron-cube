/**
 * @file
 * @brief GLFW-based keyboard input implementation for Linux.
 */

#pragma once

#include "platform/IInput.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <cstdint>

/**
 * @brief GLFW keyboard input backend.
 *
 * Tracks per-key press/release state via a GLFW key callback.
 * Uses a static singleton pattern so the C-callback can access
 * the instance.
 */
class InputGLFW
{
    public:
    /**
     * @brief Initialise input tracking.
     *
     * Registers the global key callback and resets key state.
     */
    void init();

    /**
     * @brief Clean up input resources.
     */
    void shutdown();

    /**
     * @brief Check if a key is currently pressed.
     * @param key The key to query.
     * @return true if the key is down.
     */
    bool isKeyPressed(KeyCode key) const;

    /**
     * @brief GLFW key callback handler.
     *
     * Called by GLFW on key press/release events.
     * Updates the internal key state map.
     *
     * @param window   GLFW window (unused)
     * @param key      GLFW key code
     * @param scancode Hardware scancode
     * @param action   GLFW_PRESS or GLFW_RELEASE
     * @param mods     Modifier mask
     */
    static void keyCallback(GLFWwindow *, int, int, int, int);

    private:
    static InputGLFW *instance_;
    std::unordered_map<int32_t, bool> key_states_;
};
