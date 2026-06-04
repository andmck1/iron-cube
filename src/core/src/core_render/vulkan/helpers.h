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

// TODO - move this somewhere else, need to generalise the vertex logic
static vk::VertexInputBindingDescription getVertexBindingDescription()
{
    return {0, sizeof(VertexInputType), vk::VertexInputRate::eVertex};
}

static std::array<vk::VertexInputAttributeDescription, 3>
getVertexAttributeDescriptions()
{
    return {
        {{0, 0, vk::Format::eR32G32B32Sfloat,
          offsetof(VertexInputType, position)},
         {1, 0, vk::Format::eR32G32Sfloat, offsetof(VertexInputType, tex)},
         {2, 0, vk::Format::eR32G32B32Sfloat,
          offsetof(VertexInputType, normal)}}
    };
}

