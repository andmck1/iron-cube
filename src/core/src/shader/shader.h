#pragma once

#include "shader/ishader.h"
#include <string>
#include <vulkan/vulkan.h>

class Shader : public IShader
{
    public:
    Shader(
        VkDevice device, const std::string &path, VkShaderStageFlagBits stage,
        const char *entryPoint = "main"
    );
    ~Shader() override;

    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;

    VkPipelineShaderStageCreateInfo getShaderStageInfo() const override;

    private:
    VkDevice device_;
    VkShaderModule module_ = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage_;
    const char *entryPoint_;
};
