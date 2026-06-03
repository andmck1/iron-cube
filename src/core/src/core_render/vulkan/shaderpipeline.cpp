#include "core_render/vulkan/shaderpipeline.h"
#include <spdlog/spdlog.h>

std::optional<vk::raii::Pipeline> &VulkanShaderPipeline::getGraphicsPipeline()
{
    return graphicsPipeline_;
}
