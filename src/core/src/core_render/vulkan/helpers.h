#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

inline void vulkanCheck(VkResult result, const char *message)
{
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error(
            std::string(message) + " (VkResult: " + std::to_string(result) + ")"
        );
    }
}
