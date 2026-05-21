/**
 * @file
 * @brief Platform abstraction interface for window, input, and lifecycle
 * management.
 *
 * The platform layer sits between the application and OS-specific backends,
 * providing a stable interface for window creation, event polling, and
 * graceful shutdown.
 */

#pragma once

#include <string>
#include <cstdint>

/**
 * @brief Configuration for window creation.
 *
 * Passed to IPlatform::createWindow to control the appearance
 * and size of the application window.
 */
struct PlatformConfig
{
    std::string window_title = "Iron Cube"; ///< Window title bar text
    uint32_t window_width = 1280;           ///< Window width in pixels
    uint32_t window_height = 720;           ///< Window height in pixels
};

/**
 * @brief Platform service interface.
 *
 * Abstracts OS-specific window management and event handling.
 * Implementations handle GLFW/SDL/etc. initialisation, window
 * lifecycle, and input routing.
 *
 * @par Thread safety
 * All methods must be called from the main thread.
 */
class IPlatform
{
    public:
    virtual ~IPlatform() = default;

    /**
     * @brief Initialize the platform backend.
     * @return true on success, false if the backend failed to start.
     */
    virtual bool initialise() = 0;

    /**
     * @brief Create the application window.
     * @param config Window size, title, and other properties.
     * @return true on success, false on failure.
     */
    virtual bool createWindow(const PlatformConfig &config) = 0;

    /**
     * @brief Process pending OS events (keyboard, window close, etc.).
     *
     * Call once per frame. Non-blocking.
     */
    virtual void pollEvents() = 0;

    /**
     * @brief Check whether the application should exit.
     * @return true if the window was closed or Escape was pressed.
     */
    virtual bool shouldExit() const = 0;

    /**
     * @brief Release all platform resources.
     *
     * Destroys the window and terminates the backend.
     */
    virtual void shutdown() = 0;
};
