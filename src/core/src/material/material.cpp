#include "material/material.h"
#include "render/irenderer.h"
#include "service/servicelocator.h"
#include <spdlog/spdlog.h>

Material::Material()
{
    // TODO - default should be empty and I should check for empty
    ShaderDefinition vertShader;
    vertShader.path = "build/shaders/triangle.vert.spv";
    vertShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
    config_.shaders.push_back(vertShader);

    ShaderDefinition fragShader;
    fragShader.path = "build/shaders/triangle.frag.spv";
    fragShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    config_.shaders.push_back(fragShader);

    VertexBinding binding;
    binding.binding = 0;
    binding.stride = 24; // 2 x vec3 (position + color)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    config_.vertexBindings.push_back(binding);

    VertexAttribute attrPos;
    attrPos.location = 0;
    attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrPos.offset = 0;
    config_.vertexAttributes.push_back(attrPos);

    VertexAttribute attrColor;
    attrColor.location = 1;
    attrColor.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrColor.offset = 12; // sizeof(glm::vec3)
    config_.vertexAttributes.push_back(attrColor);

    DescriptorBinding uboBinding;
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    config_.descriptorBindings.push_back(uboBinding);
}

Material::Material(ShaderPipelineConfig config) : config_(std::move(config))
{
}

void Material::bind()
{
    spdlog::debug("Binding material");
}

void Material::setUniform(std::string matrixName, glm::mat4 matrix)
{
    spdlog::debug("Setting uniform: {}", matrixName);
    IRenderer *renderer = ServiceLocator::instance().get<IRenderer>();
    if (!renderer)
        return;

    if (matrixName == "modelMatrix")
    {
        renderer->setUniformModelMatrix(matrix);
    }
}

ShaderPipelineConfig Material::getConfig() const
{
    return config_;
}
