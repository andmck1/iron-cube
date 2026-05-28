#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

inline void vkCheck(VkResult result, const char *message)
{
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error(
            std::string(message) + " (VkResult: " + std::to_string(result) + ")"
        );
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData
);

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance);

void destroyDebugMessenger(
    VkInstance instance, VkDebugUtilsMessengerEXT messenger
);


