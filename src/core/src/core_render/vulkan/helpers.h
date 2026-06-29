#pragma once

#include "core_buffertypes/buffertypes.h"
#include <vulkan/vulkan_raii.hpp>
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

static bool hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint ||
           format == vk::Format::eD24UnormS8Uint;
}
