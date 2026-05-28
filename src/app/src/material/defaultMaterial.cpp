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

DefaultMaterial::bind()
{
    UBO ubo{};
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    std::memcpy(ubo.view, glm::value_ptr(view), 16 * sizeof(float));

    float aspect =
        swapchainExtent_.width / static_cast<float>(swapchainExtent_.height);
    glm::mat4 proj =
        glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    proj[1][1] *= -1.0f;
    std::memcpy(ubo.proj, glm::value_ptr(proj), 16 * sizeof(float));

    void *uboMapped = mapMemory(uniformAlloc_, sizeof(UBO));
    if (uboMapped)
    {
        std::memcpy(uboMapped, &ubo, sizeof(UBO));
        unmapMemory(uniformAlloc_);
    }
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
