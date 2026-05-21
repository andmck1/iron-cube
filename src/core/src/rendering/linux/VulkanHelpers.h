/**
 * @file
 * @brief Vulkan utility functions and validation layer setup.
 *
 * Provides inline helper for VkResult checking and debug utils
 * messenger callback for validation layer logging.
 */

#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

/**
 * @brief Check a VkResult and throw on failure.
 * @param result Vulkan result code.
 * @param message Error message to include in exception.
 * @throws std::runtime_error if result is not VK_SUCCESS or VK_SUBOPTIMAL.
 */
inline void vkCheck(VkResult result, const char *message)
{
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error(
            std::string(message) + " (VkResult: " + std::to_string(result) + ")"
        );
    }
}

/**
 * @brief Debug utils messenger callback for validation layer logging.
 *
 * Routes validation layer messages through spdlog with appropriate
 * severity levels.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData
);

/**
 * @brief Create a debug utils messenger for validation layer logging.
 * @param instance Vulkan instance handle.
 * @return Messenger handle, or VK_NULL_HANDLE if validation layers unavailable.
 */
VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance);

/**
 * @brief Destroy a debug utils messenger.
 * @param instance Vulkan instance handle.
 * @param messenger Messenger handle to destroy.
 */
void destroyDebugMessenger(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger
);
