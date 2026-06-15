#pragma once

#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <vulkan/vulkan_raii.hpp>

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    std::optional<vk::raii::Pipeline> graphicsPipeline_;
    ShaderPipelineUpdateData *updateData_ = nullptr;
};
