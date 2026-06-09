#include "default.h"
#include "core_buffertypes/buffertypes.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/helpers.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_clip_space.hpp"

bool DefaultVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    geometry_ = geometry;
    updateData_ = updateData;
    return initPipeline();
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool DefaultVulkanShaderPipeline::initPipeline()
{
    if (!initDescriptorLayout())
        return false;
    if (!initUniformBuffers())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
        return false;
    if (!initVertexBuffer())
        return false;
    if (!initIndexBuffer())
        return false;
    return true;
}

bool DefaultVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool DefaultVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    vertexBuffer_ = nullptr;
    vertexBufferMemory_ = nullptr;
    indexBuffer_ = nullptr;
    indexBufferMemory_ = nullptr;
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

    vk::PipelineDepthStencilStateCreateInfo depthStencil =
        vk::PipelineDepthStencilStateCreateInfo{}
            .setDepthTestEnable(vk::True)
            .setDepthWriteEnable(vk::True)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(vk::False)
            .setStencilTestEnable(vk::False);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount(1)
            .setPSetLayouts(&*descriptorSetLayout_)
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
            .setPDepthStencilState(&depthStencil)
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

bool DefaultVulkanShaderPipeline::initVertexBuffer()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const std::vector<VertexInputType> &vertices = geometry_->vertices;

    vk::DeviceSize bufferSize = sizeof(VertexInputType) * vertices.size();
    auto [stagingBuffer, stagingBufferMemory] = renderer->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, vertices.data(), bufferSize);
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

bool DefaultVulkanShaderPipeline::initIndexBuffer()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const std::vector<uint16_t> &indices = geometry_->indices;

    vk::DeviceSize bufferSize = sizeof(uint16_t) * indices.size();
    auto [stagingBuffer, stagingBufferMemory] = renderer->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(dataStaging, indices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(indexBuffer_, indexBufferMemory_) = renderer->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    renderer->copyBuffer(stagingBuffer, indexBuffer_, bufferSize);

    return true;
}

bool DefaultVulkanShaderPipeline::initDescriptorLayout()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::DescriptorSetLayoutBinding mvpLayoutBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo layoutInfo =
        vk::DescriptorSetLayoutCreateInfo{}.setBindingCount(1).setPBindings(
            &mvpLayoutBinding
        );
    descriptorSetLayout_ =
        vk::raii::DescriptorSetLayout(renderer->getDevice(), layoutInfo);
    return true;
}

bool DefaultVulkanShaderPipeline::initUniformBuffers()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DeviceSize bufferSize = sizeof(MVPBuffer);
        auto [buffer, bufferMem] = renderer->createBuffer(
            bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent
        );
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemory.emplace_back(std::move(bufferMem));
        uniformBuffersMapped.emplace_back(
            uniformBuffersMemory.back().mapMemory(0, bufferSize)
        );
    }

    return true;
}

bool DefaultVulkanShaderPipeline::createDescriptorPool()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(renderer->MAX_FRAMES_IN_FLIGHT)
    };

    vk::DescriptorPoolCreateInfo poolInfo =
        vk::DescriptorPoolCreateInfo{}
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(renderer->MAX_FRAMES_IN_FLIGHT)
            .setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
            .setPPoolSizes(poolSizes.data());

    descriptorPool_ = vk::raii::DescriptorPool(renderer->getDevice(), poolInfo);
    return true;
}

bool DefaultVulkanShaderPipeline::initDescriptorSets()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    if (!createDescriptorPool())
        return false;

    std::vector<vk::DescriptorSetLayout> layouts(
        renderer->MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout_
    );
    vk::DescriptorSetAllocateInfo allocInfo =
        vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(descriptorPool_)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

    descriptorSets_ = renderer->getDevice().allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo{}
                                                  .setBuffer(uniformBuffers[i])
                                                  .setOffset(0)
                                                  .setRange(sizeof(MVPBuffer));
        vk::WriteDescriptorSet descriptorWrite =
            vk::WriteDescriptorSet{}
                .setDstSet(descriptorSets_[i])
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setPBufferInfo(&bufferInfo);
        renderer->getDevice().updateDescriptorSets(descriptorWrite, {});
    }
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
    commandBuffer.bindIndexBuffer(*indexBuffer_, 0, vk::IndexType::eUint16);
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipelineLayout_, 0,
        *descriptorSets_[renderer->getCurrentFrameIndex()], nullptr
    );
    commandBuffer.setViewport(0, renderer->getViewport());
    commandBuffer.setScissor(
        0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent)
    );
    spdlog::debug("Finished binding shader pipeline");

    return true;
}

bool DefaultVulkanShaderPipeline::update()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    const DefaultShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const DefaultShaderPipelineUpdateData &>(*updateData_);

    MVPBuffer mvp{};
    mvp.model = *defaultUpdateData.actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *defaultUpdateData.camera->getViewMatrixRef();
    mvp.projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(renderer->getSwapChainExtent().width) /
            static_cast<float>(renderer->getSwapChainExtent().height),
        0.1f, 10.0f
    );
    mvp.projection[1][1] *= -1;
    memcpy(
        uniformBuffersMapped[renderer->getCurrentFrameIndex()], &mvp,
        sizeof(mvp)
    );
    return true;
}

bool DefaultVulkanShaderPipeline::draw()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    renderer->getCommandBuffer().drawIndexed(
        static_cast<uint32_t>(geometry_->indices.size()), 1, 0, 0, 0
    );
    return true;
}
