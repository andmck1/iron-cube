#include "material/material.h"
#include "render/irenderer.h"
#include "service/servicelocator.h"
#include <spdlog/spdlog.h>

Material::Material()
{
    // TODO - default should be empty and I should check for empty
    pipeline_.vertexShaderPath = "build/shaders/triangle.vert.spv";
    pipeline_.fragmentShaderPath = "build/shaders/triangle.frag.spv";

    VertexBinding binding;
    binding.binding = 0;
    binding.stride = 24; // 2 x vec3 (position + color)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    pipeline_.vertexBindings.push_back(binding);

    VertexAttribute attrPos;
    attrPos.location = 0;
    attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrPos.offset = 0;
    pipeline_.vertexAttributes.push_back(attrPos);

    VertexAttribute attrColor;
    attrColor.location = 1;
    attrColor.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrColor.offset = 12; // sizeof(glm::vec3)
    pipeline_.vertexAttributes.push_back(attrColor);

    DescriptorBinding uboBinding;
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pipeline_.descriptorBindings.push_back(uboBinding);
}

Material::Material(ShaderPipeline pipeline) : pipeline_(std::move(pipeline))
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

ShaderPipeline Material::getPipeline() const
{
    return pipeline_;
}
