// TODO - material folder conflict
#include "defaultMaterial.h"

DefaultMaterial::DefaultMaterial()
{
    ShaderDefinition vertShader;
    vertShader.path = "build/shader/slang/vertex/triangle.vert.spv";
    vertShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
    config_.shaders.push_back(vertShader);

    ShaderDefinition fragShader;
    fragShader.path = "build/shader/slang/fragment/triangle.frag.spv";
    fragShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    config_.shaders.push_back(fragShader);

    VertexBinding binding;
    binding.binding = 0;
    binding.stride = 24; // 2 x vec3 (position + colour)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    config_.vertexBindings.push_back(binding);

    VertexAttribute attrPos;
    attrPos.location = 0;
    attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrPos.offset = 0;
    config_.vertexAttributes.push_back(attrPos);

    VertexAttribute attrColour;
    attrColour.location = 1;
    attrColour.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrColour.offset = 12; // sizeof(glm::vec3)
    config_.vertexAttributes.push_back(attrColour);

    DescriptorBinding uboBinding;
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    config_.descriptorBindings.push_back(uboBinding);
}

CubeMaterial::CubeMaterial()
{
    ShaderDefinition vertShader;
    vertShader.path = "build/shader/slang/vertex/triangle.vert.spv";
    vertShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
    config_.shaders.push_back(vertShader);

    ShaderDefinition fragShader;
    fragShader.path = "build/shader/slang/fragment/triangle.frag.spv";
    fragShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    config_.shaders.push_back(fragShader);

    VertexBinding binding;
    binding.binding = 0;
    binding.stride = 24; // 2 x vec3 (position + colour)
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    config_.vertexBindings.push_back(binding);

    VertexAttribute attrPos;
    attrPos.location = 0;
    attrPos.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrPos.offset = 0;
    config_.vertexAttributes.push_back(attrPos);

    VertexAttribute attrColour;
    attrColour.location = 1;
    attrColour.format = VK_FORMAT_R32G32B32_SFLOAT;
    attrColour.offset = 12; // sizeof(glm::vec3)
    config_.vertexAttributes.push_back(attrColour);

    DescriptorBinding uboBinding;
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    config_.descriptorBindings.push_back(uboBinding);
}
