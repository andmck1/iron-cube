/**
 * @file
 * @brief Window abstraction interface.
 *
 * Provides a platform-agnostic view over window state and
 * lifecycle operations.
 */

#pragma once

#include <cstdint>

/**
 * @brief Unsigned 2D vector for window dimensions.
 */
struct Vec2u
{
    uint32_t x; ///< Width or X coordinate
    uint32_t y; ///< Height or Y coordinate
};

/**
 * @brief Window abstraction interface.
 *
 * Exposes window state queries and close requests without
 * exposing backend-specific handles.
 */
class IWindow
{
    public:
    virtual ~IWindow() = default;

    /**
     * @brief Check if the window is currently open.
     * @return true if the window has been created and not destroyed.
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Check if the window is minimized.
     * @return true if the window is iconified.
     */
    virtual bool isIconified() const = 0;

    /**
     * @brief Signal the window to close.
     *
     * Sets the close flag; the actual destruction happens
     * during the next platform shutdown cycle.
     */
    virtual void requestClose() = 0;

    /**
     * @brief Get the current window size.
     * @return Width and height in pixels.
     */
    virtual Vec2u getSize() const = 0;
};
