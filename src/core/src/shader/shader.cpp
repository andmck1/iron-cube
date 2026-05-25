#include "shader/shader.h"
#include "render/linux/vulkanhelpers.h"
#include <spdlog/spdlog.h>
#include <fstream>

Shader::Shader(
    VkDevice device, const std::string &path, VkShaderStageFlagBits stage,
    const char *entryPoint
)
    : device_(device), stage_(stage), entryPoint_(entryPoint)
{
    spdlog::debug(
        "Loading shader: {} (stage: {})", path, static_cast<uint32_t>(stage)
    );

    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file: " + path);
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

Shader::~Shader()
{
    if (module_ != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device_, module_, nullptr);
        module_ = VK_NULL_HANDLE;
    }
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo() const
{
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage_;
    stageInfo.module = module_;
    stageInfo.pName = entryPoint_;
    return stageInfo;
}
