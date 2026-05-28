#pragma once

#include "core_material/imaterial.h"
#include "core_shader/vulkanshader.h"
#include <vector>

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

struct DynamicState
{
    VkDynamicState state;
};

struct VulkanMaterialConfig : BaseMaterialConfig
{
    std::vector<VertexBinding> vertexBindings;
    std::vector<VertexAttribute> vertexAttributes;

    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
};

class VulkanMaterial : public IMaterial
{
    VulkanMaterial(VulkanMaterialConfig materialConfig);

    VulkanMaterialConfig *getConfig();
    bool init() override;
    bool set() override;
    void addShader(IShader *shader) override;

    protected:
    bool createGraphicsPipeline();
    void createUniformBuffer();
    void createDescriptorSet();

    VulkanMaterialConfig config_;
    VkDescriptorSet descriptorSet_{};
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
    std::vector<VulkanShader *> shaders_;
};
