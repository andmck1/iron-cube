#include "shader/shaderpipeline.h"
#include "render/linux/vulkanhelpers.h"
#include <spdlog/spdlog.h>
#include <fstream>

ShaderPipeline::ShaderPipeline(
    VkDevice device, const std::string &vertPath, const std::string &fragPath
)
    : device_(device)
{
    spdlog::debug("Loading shaders: {} -> {}", vertPath, fragPath);
    vertModule_ = createShaderModule(device_, vertPath);
    fragModule_ = createShaderModule(device_, fragPath);
}

ShaderPipeline::~ShaderPipeline()
{
    if (vertModule_ != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device_, vertModule_, nullptr);
        vertModule_ = VK_NULL_HANDLE;
    }
    if (fragModule_ != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device_, fragModule_, nullptr);
        fragModule_ = VK_NULL_HANDLE;
    }
}

VkShaderModule ShaderPipeline::getVertexModule() const
{
    return vertModule_;
}

VkShaderModule ShaderPipeline::getFragmentModule() const
{
    return fragModule_;
}

VkShaderModule ShaderPipeline::createShaderModule(
    VkDevice device, const std::string &path
) const
{
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

    VkShaderModule shaderModule;
    VkResult result =
        vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    vkCheck(result, "Failed to create shader module");
    return shaderModule;
}
