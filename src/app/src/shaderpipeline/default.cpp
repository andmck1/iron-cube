#include "default.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/helpers.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"

bool DefaultVulkanShaderPipeline::init(Geometry *geometry)
{
    if (!initGraphicsPipeline())
        return false;
    if (!initVertexBuffer(geometry))
        return false;
    return true;
}

bool DefaultVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    vertexBuffer_ = nullptr;
    vertexBufferMemory_ = nullptr;
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    spdlog::debug("Shutdown complete for default vulkan pipeline");
    return true;
}

bool DefaultVulkanShaderPipeline::initGraphicsPipeline()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *defaultVertexShader =
        dynamic_cast<VulkanShader *>(shaderManager->getShader("defaultVertex"));
    VulkanShader *defaultFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("defaultFragment")
    );

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(defaultVertexShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(defaultFragmentShader->getShaderModule())
            .setPName("main");

    vk::VertexInputBindingDescription bindingDescription =
        getVertexBindingDescription();
    auto attributeDescriptions = getVertexAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo =
        vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(1)
            .setPVertexBindingDescriptions(&bindingDescription)
            .setVertexAttributeDescriptionCount(
                static_cast<uint32_t>(attributeDescriptions.size())
            )
            .setPVertexAttributeDescriptions(attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly =
        renderer->getInputAssembly();

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderStageInfo, fragShaderStageInfo
    };

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState =
        vk::PipelineDynamicStateCreateInfo{}
            .setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
            .setPDynamicStates(dynamicStates.data());

    vk::PipelineViewportStateCreateInfo viewportState =
        vk::PipelineViewportStateCreateInfo{}
            .setViewportCount(1)
            .setScissorCount(1);

    vk::PipelineRasterizationStateCreateInfo rasteriser =
        renderer->getRasteriser();

    vk::PipelineMultisampleStateCreateInfo multisampling =
        renderer->getMultisampling();

    vk::PipelineColorBlendAttachmentState colourBlendAttachment =
        renderer->getColourBlendAttachment();

    vk::PipelineColorBlendStateCreateInfo colourBlending =
        vk::PipelineColorBlendStateCreateInfo{}
            .setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1)
            .setPAttachments(&colourBlendAttachment);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount(0)
            .setPushConstantRangeCount(0);

    pipelineLayout_ =
        vk::raii::PipelineLayout(renderer->getDevice(), pipelineLayoutInfo);

    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
        vk::GraphicsPipelineCreateInfo{}
            .setStageCount(2)
            .setPStages(shaderStages)
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&inputAssembly)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasteriser)
            .setPMultisampleState(&multisampling)
            .setPColorBlendState(&colourBlending)
            .setPDynamicState(&dynamicState)
            .setLayout(pipelineLayout_)
            .setRenderPass(nullptr);

    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo =
        renderer->getPipelineRendereringCreateInfo();

    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo>
        pipelineCreateInfoChain = {
            graphicsPipelineCreateInfo, pipelineRenderingCreateInfo
        };

    graphicsPipeline_ = vk::raii::Pipeline(
        renderer->getDevice(), nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
    return true;
}

bool DefaultVulkanShaderPipeline::initVertexBuffer(Geometry *geometry)
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    std::shared_ptr<std::vector<VertexInputType>> vertices = geometry->vertices;

    vk::DeviceSize bufferSize = sizeof(vertices->front()) * vertices->size();
    auto [stagingBuffer, stagingBufferMemory] = renderer->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices->data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(vertexBuffer_, vertexBufferMemory_) = renderer->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    renderer->copyBuffer(stagingBuffer, vertexBuffer_, bufferSize);

    return true;
}

bool DefaultVulkanShaderPipeline::bind()
{
    spdlog::debug("Binding shader pipeline");

    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    vk::raii::CommandBuffer &commandBuffer = renderer->getCommandBuffer();
    vk::Extent2D &swapChainExtent = renderer->getSwapChainExtent();

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *getGraphicsPipeline()
    );
    commandBuffer.bindVertexBuffers(0, *vertexBuffer_, {0});
    commandBuffer.setViewport(
        0, vk::Viewport(
               0.0f, 0.0f, static_cast<float>(swapChainExtent.width),
               static_cast<float>(swapChainExtent.height), 0.0f, 1.0f
           )
    );
    commandBuffer.setScissor(
        0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent)
    );
    spdlog::debug("Finished binding shader pipeline");

    return true;
}
