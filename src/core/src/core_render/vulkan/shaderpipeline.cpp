#include "core_render/vulkan/shaderpipeline.h"

std::optional<vk::raii::Pipeline> &VulkanShaderPipeline::getGraphicsPipeline()
{
    return graphicsPipeline_;
}
