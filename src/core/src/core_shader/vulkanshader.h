#pragma once

#include "core_buffer/vulkanbuffer.h"
#include "core_shader/ishader.h"

#include <optional>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

struct DescriptorBinding
{
    uint32_t binding = 0;
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uint32_t descriptorCount = 1;
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
};

struct VulkanShaderConfig : public BaseShaderConfig
{
    VkDevice device = nullptr;
    std::string path = "";
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    const char *entryPoint = "main";
    std::vector<DescriptorBinding> descriptorBindings;
    std::optional<VkPushConstantRange> pushConstantRange;
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
    VkDescriptorSetLayout getDescriptorSetLayout() const;
    std::optional<VkPushConstantRange> getPushConstantRange() const;

    protected:
    void createDescriptorSetLayout();

    VulkanShaderConfig config_;
    VkDevice device_;
    VkShaderModule module_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout_;
    std::vector<std::unique_ptr<VulkanBuffer>> buffers_;
};
