#include "core_material/vulkanmaterial.h"
#include "core_render/linux/vulkanhelpers.h"
#include "core_render/linux/vulkanrenderer.h"
#include "core_service/servicelocator.h"
#include "core_shader/vulkanshader.h"
#include "glm/ext/matrix_float4x4.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

VulkanMaterial::VulkanMaterial(VulkanMaterialConfig materialConfig)
{
    config_ = materialConfig;
}

bool VulkanMaterial::init()
{
    spdlog::debug("Initialising graphics pipelines");
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    try
    {
        createGraphicsPipeline();
        createUniformBuffer();
        createDescriptorSet();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Pipeline initialisation failed: {}", e.what());
        return false;
    }

    spdlog::info("Graphics pipelines initialised");
    return true;
}

bool VulkanMaterial::createGraphicsPipeline()
{
    spdlog::debug("Creating graphics pipeline");

    VulkanRenderer *renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const VkDevice device_ = renderer_->getDevice();
    const VkRenderPass renderPass_ = renderer_->getRenderPass();

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (const IShader *shader : shaders_)
    {
        const VulkanShader *vulkanShader =
            dynamic_cast<const VulkanShader *>(shader);
        shaderStages.push_back(vulkanShader->getShaderStageInfo());
    }

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    for (const VertexBinding &vb : config_.vertexBindings)
    {
        VkVertexInputBindingDescription desc{};
        desc.binding = vb.binding;
        desc.stride = vb.stride;
        desc.inputRate = vb.inputRate;
        bindingDescriptions.push_back(desc);
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    for (const VertexAttribute &va : config_.vertexAttributes)
    {
        VkVertexInputAttributeDescription desc{};
        desc.binding = 0;
        desc.location = va.location;
        desc.format = va.format;
        desc.offset = va.offset;
        attributeDescriptions.push_back(desc);
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInput.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInput.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = config_.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasteriser{};
    rasteriser.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasteriser.depthClampEnable = VK_FALSE;
    rasteriser.rasterizerDiscardEnable = VK_FALSE;
    rasteriser.polygonMode = config_.polygonMode;
    rasteriser.lineWidth = 1.0f;
    rasteriser.cullMode = config_.cullMode;
    rasteriser.frontFace = config_.frontFace;
    rasteriser.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = config_.samples;

    VkPipelineColorBlendAttachmentState colourBlendAttachment{};
    colourBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colourBlending{};
    colourBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlending.logicOpEnable = VK_FALSE;
    colourBlending.attachmentCount = 1;
    colourBlending.pAttachments = &colourBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(config_.dynamicStates.size());
    dynamicState.pDynamicStates = config_.dynamicStates.data();

    // Push constant range for per-draw model matrix (64 bytes =
    // sizeof(glm::mat4))
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkPushConstantRange> pushConstantRanges;
    for (VulkanShader *shader : shaders_)
    {
        descriptorSetLayouts.push_back(shader->getDescriptorSetLayout());
        if (shader->getPushConstantRange())
        {
            pushConstantRanges.push_back(
                shader->getPushConstantRange().value()
            );
        }
    }
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount =
        static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (pushConstantRanges.size() > 0)
    {
        pipelineLayoutInfo.pushConstantRangeCount =
            static_cast<uint32_t>(pushConstantRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    }

    VkResult result = vkCreatePipelineLayout(
        device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_
    );
    vkCheck(result, "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasteriser;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colourBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.layout = pipelineLayout_;

    result = vkCreateGraphicsPipelines(
        device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_
    );
    vkCheck(result, "Failed to create graphics pipeline");
    return true;
}

VulkanMaterialConfig *VulkanMaterial::getConfig()
{
    return dynamic_cast<VulkanMaterialConfig *>(&config_);
}

bool VulkanMaterial::set()
{
    for (VulkanShader *shader : shaders_)
    {
       shader->set(); 
    }

    renderer_->bindPipeline(graphicsPipeline_);
    renderer_->bindDescriptorSet(pipelineLayout_, descriptorSet_);
    return true;
}

void VulkanMaterial::addShader(IShader *shader)
{
    VulkanShader *vulkanShader = dynamic_cast<VulkanShader *>(shader);
    shaders_.push_back(vulkanShader);
}
