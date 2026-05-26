#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

struct VertexAttribute
{
    uint32_t location = 0;
    VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
    VkDeviceSize offset = 0;
};

struct VertexBinding
{
    uint32_t binding = 0;
    VkDeviceSize stride = 0;
    VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
};

struct DescriptorBinding
{
    uint32_t binding = 0;
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uint32_t descriptorCount = 1;
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
};

struct DynamicState
{
    VkDynamicState state;
};

struct ShaderDefinition
{
    std::string path;
    VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    const char *entryPoint = "main";
};

struct ShaderPipelineConfig
{
    std::vector<ShaderDefinition> shaders;

    std::vector<VertexBinding> vertexBindings;
    std::vector<VertexAttribute> vertexAttributes;

    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<DescriptorBinding> descriptorBindings;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
};

class Material
{
    public:
    Material();
    explicit Material(ShaderPipelineConfig config);

    void bind();
    void setUniform(std::string matrixName, glm::mat4 matrix);

    ShaderPipelineConfig getConfig() const;

    protected:
    ShaderPipelineConfig config_;
};

class DefaultMaterial : public Material
{
    public:
    DefaultMaterial();
};

class CubeMaterial : public Material
{
    public:
    CubeMaterial();
};
