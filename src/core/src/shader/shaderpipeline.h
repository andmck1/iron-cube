#pragma once

#include <string>
#include <vulkan/vulkan.h>

class ShaderPipeline
{
    public:
    ShaderPipeline(
        VkDevice device, const std::string &vertPath,
        const std::string &fragPath
    );
    ~ShaderPipeline();

    ShaderPipeline(const ShaderPipeline &) = delete;
    ShaderPipeline &operator=(const ShaderPipeline &) = delete;

    VkShaderModule getVertexModule() const;
    VkShaderModule getFragmentModule() const;

    private:
    VkShaderModule
    createShaderModule(VkDevice device, const std::string &path) const;

    VkDevice device_;
    VkShaderModule vertModule_ = VK_NULL_HANDLE;
    VkShaderModule fragModule_ = VK_NULL_HANDLE;
};
