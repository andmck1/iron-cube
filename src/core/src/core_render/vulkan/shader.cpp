#include "core_render/vulkan/shader.h"
#include "core_render/irenderer.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"

#include <spdlog/spdlog.h>
#include <fstream>

VulkanShader::VulkanShader(const VulkanShaderConfig shaderConfig)
{
    spdlog::debug("Loading shader: {}", shaderConfig.path);
    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    VulkanRenderer *vulkanRenderer = dynamic_cast<VulkanRenderer *>(renderer);

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
    shaderModule_.emplace(
        vk::raii::ShaderModule(vulkanRenderer->getDevice(), createInfo)
    );

    spdlog::debug("Finished loading shader: {}", shaderConfig.path);
}


std::optional<vk::raii::ShaderModule> *VulkanShader::getShaderModule()
{
    return &shaderModule_;
}
