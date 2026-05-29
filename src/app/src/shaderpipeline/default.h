#pragma once

#include "core_render/ishaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultVulkanShaderPipeline : public IShaderPipeline
{
    public:
    DefaultVulkanShaderPipeline();

    protected:
    vk::Pipeline graphicsPipeline_ = nullptr;
};
