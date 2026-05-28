#pragma once

#include "core_buffer/vulkanbuffer.h"
#include "core_shader/ishader.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

struct VulkanShaderConfig : public BaseShaderConfig
{
    VkDevice device = nullptr;
    std::string path = "";
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    const char *entryPoint = "main";
};

class VulkanShader : public IShader
{
    public:
    VulkanShader();
    ~VulkanShader() override;

    VulkanShader(const VulkanShader &) = delete;
    VulkanShader &operator=(const VulkanShader &) = delete;

    void init(const BaseShaderConfig &shaderConfig) override;
    IBuffer *addBuffer(IBuffer *buffer) override;
    void set() override;

    VkPipelineShaderStageCreateInfo getShaderStageInfo() const;

    protected:
    VkDevice device_;
    VkShaderModule module_ = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage_;
    const char *entryPoint_;
    std::vector<std::unique_ptr<VulkanBuffer>> buffers_;
};
