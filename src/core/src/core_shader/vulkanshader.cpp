#include "core_shader/vulkanshader.h"
#include "core_render/linux/vulkanhelpers.h"
#include <spdlog/spdlog.h>
#include <fstream>

VulkanShader::VulkanShader()
{
}

VulkanShader::~VulkanShader()
{
    if (module_ != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device_, module_, nullptr);
        module_ = VK_NULL_HANDLE;
    }
}

void VulkanShader::init(const BaseShaderConfig &baseShaderConfig)
{
    const VulkanShaderConfig &shaderConfig =
        dynamic_cast<const VulkanShaderConfig &>(baseShaderConfig);
    config_ = shaderConfig;

    spdlog::debug(
        "Loading shader: {} (stage: {})", shaderConfig.path,
        static_cast<uint32_t>(shaderConfig.stage)
    );

    std::ifstream file(shaderConfig.path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error(
            "Failed to open shader file: " + shaderConfig.path
        );
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());

    VkResult result =
        vkCreateShaderModule(device_, &createInfo, nullptr, &module_);
    vkCheck(result, "Failed to create shader module");
}
IBuffer *VulkanShader::addBuffer(IBuffer *buffer)
{
    VulkanBuffer *vulkanBuffer = dynamic_cast<VulkanBuffer *>(buffer);
    buffers_.push_back(std::unique_ptr<VulkanBuffer>(vulkanBuffer));
    return buffers_.back().get();
}

void VulkanShader::createDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for (const DescriptorBinding &descriptorBinding :
         config_.descriptorBindings)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = descriptorBinding.binding;
        binding.descriptorType = descriptorBinding.descriptorType;
        binding.descriptorCount = descriptorBinding.descriptorCount;
        binding.stageFlags = descriptorBinding.stageFlags;
        binding.pImmutableSamplers = nullptr;
        layoutBindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    vkCheck(
        vkCreateDescriptorSetLayout(
            device_, &layoutInfo, nullptr, &descriptorSetLayout_
        ),
        "Failed to create descriptor set layout"
    );
}

VkPipelineShaderStageCreateInfo VulkanShader::getShaderStageInfo() const
{
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = config_.stage;
    stageInfo.module = module_;
    stageInfo.pName = config_.entryPoint;
    return stageInfo;
}

VkDescriptorSetLayout VulkanShader::getDescriptorSetLayout() const
{
    return descriptorSetLayout_;
}

std::optional<VkPushConstantRange> VulkanShader::getPushConstantRange() const
{
    return config_.pushConstantRange;
}

void VulkanShader::set()
{
    for (const std::unique_ptr<VulkanBuffer> &buffer : buffers_)
    {
        buffer->set();
    }
}
