/**
 * @file
 * @brief Input abstraction interface for keyboard state.
 */

#pragma once

#include <cstdint>

/**
 * @brief Keyboard key codes.
 *
 * Values map to GLFW key constants for the Linux backend.
 * Extend as needed for additional keys.
 */
enum class KeyCode : int32_t
{
    Escape = 256,  ///< Escape / Esc key
};

/**
 * @brief Input abstraction interface.
 *
 * Provides frame-accurate keyboard state queries.
 * The backend updates key state during event polling.
 */
class IInput
{
    public:
    virtual ~IInput() = default;

    /**
     * @brief Check if a key is currently pressed.
     * @param key The key to query.
     * @return true if the key is down this frame.
     */
    virtual bool isKeyPressed(KeyCode key) const = 0;
};
