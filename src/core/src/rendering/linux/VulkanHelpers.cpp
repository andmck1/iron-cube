#include "rendering/linux/VulkanHelpers.h"
#include <functional>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

VKAPI_ATTR VkBool32 VKAPI_CALL
debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                              VkDebugUtilsMessageTypeFlagsEXT /*types*/,
                              const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                              void* /*userData*/)
{
    const char* message = callbackData ? callbackData->pMessage : "unknown";

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        spdlog::error("[Vulkan Validation] {}", message);
    }
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        spdlog::warn("[Vulkan Validation] {}", message);
    }
    else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        spdlog::info("[Vulkan Validation] {}", message);
    }
    else
    {
        spdlog::debug("[Vulkan Validation] {}", message);
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance)
{
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

    PFN_vkCreateDebugUtilsMessengerEXT pfnCreate =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (!pfnCreate)
    {
        return VK_NULL_HANDLE;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugUtilsMessengerCallback;

    if (pfnCreate(instance, &createInfo, nullptr, &messenger) != VK_SUCCESS)
    {
        spdlog::warn("Failed to create debug utils messenger");
        return VK_NULL_HANDLE;
    }

    return messenger;
}

void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
    if (messenger == VK_NULL_HANDLE)
    {
        return;
    }

    PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroy =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (pfnDestroy)
    {
        pfnDestroy(instance, messenger, nullptr);
    }
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
