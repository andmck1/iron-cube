#pragma once

#include "core_render/ishader.h"
#include <optional>
#include <vulkan/vulkan_raii.hpp>

struct VulkanShaderConfig : public BaseShaderConfig
{
    std::string path = "";
};

class VulkanShader : public IShader
{
    public:
    VulkanShader(const VulkanShaderConfig config); 

    vk::raii::ShaderModule &getShaderModule();

    protected:
    std::optional<vk::raii::ShaderModule> shaderModule_;
};
