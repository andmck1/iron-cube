#include "shaderpipeline/instancedVertex.h"
#include "core_buffertypes/buffertypes.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"

bool InstancedVertexVulkanShaderPipeline::init(
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
bool InstancedVertexVulkanShaderPipeline::initPipeline()
{
    initUniformBuffers();
    initStorageBuffers();
    if (!initDescriptorLayout())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
        return false;
    if (!initVertexBuffer())
        return false;
    if (!initIndexBuffer())
        return false;
    if (!initInstanceBuffer())
        return false;
    return true;
}

bool InstancedVertexVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedVertexVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down instanced vulkan pipeline");
    vertexBuffer_ = nullptr;
    vertexBufferMemory_ = nullptr;
    indexBuffer_ = nullptr;
    indexBufferMemory_ = nullptr;
    instanceBufferMemory_ = nullptr;
    instanceBuffer_ = nullptr;
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexVulkanShaderPipeline::initUniformBuffers()
{
    addUniformBuffer(
        sizeof(MVPBuffer), static_cast<MVPBuffer *>(nullptr),
        vk::ShaderStageFlagBits::eVertex
    );
}

void InstancedVertexVulkanShaderPipeline::initStorageBuffers()
{
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();

    addStorageRWBuffer(
        debugDataBufferSize, debugDataBufferData_.data(),
        vk::ShaderStageFlagBits::eAll 
    );
}

bool InstancedVertexVulkanShaderPipeline::initGraphicsPipeline()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVertex")
    );
    VulkanShader *defaultFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("defaultFragment")
    );

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(instancedVertexShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(defaultFragmentShader->getShaderModule())
            .setPName("main");

    std::array<vk::VertexInputBindingDescription, 2> bindingDescriptions = {
        vk::VertexInputBindingDescription{}
            .setBinding(0)
            .setStride(sizeof(VertexInputType))
            .setInputRate(vk::VertexInputRate::eVertex),
        vk::VertexInputBindingDescription{}
            .setBinding(1)
            .setStride(sizeof(Instance))
            .setInputRate(vk::VertexInputRate::eInstance),
    };
    std::array<vk::VertexInputAttributeDescription, 6> attributeDescriptions = {
        // vertex
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(0)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(VertexInputType, position)),
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(1)
            .setFormat(vk::Format::eR32G32Sfloat)
            .setOffset(offsetof(VertexInputType, tex)),
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(2)
            .setFormat(vk::Format::eR32G32B32Sfloat)
            .setOffset(offsetof(VertexInputType, normal)),
        // instance
        vk::VertexInputAttributeDescription{}
            .setBinding(1)
            .setLocation(3)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(Instance, position)),
        vk::VertexInputAttributeDescription{}
            .setBinding(1)
            .setLocation(4)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(Instance, rotation)),
        vk::VertexInputAttributeDescription{}
            .setBinding(1)
            .setLocation(5)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(Instance, scale)),
    };
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo =
        vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(2)
            .setPVertexBindingDescriptions(bindingDescriptions.data())
            .setVertexAttributeDescriptionCount(
                static_cast<uint32_t>(attributeDescriptions.size())
            )
            .setPVertexAttributeDescriptions(attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly =
        renderer->getInputAssembly();

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
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

    // Saw this note in vulkan github code:
    // Using reversed depth-buffer for increased precision,
    // so Greater depth values are kept
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
            .setStageCount(shaderStages.size())
            .setPStages(shaderStages.data())
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

// TODO - use addBuffer
bool InstancedVertexVulkanShaderPipeline::initVertexBuffer()
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

// TODO - use addBuffer
bool InstancedVertexVulkanShaderPipeline::initIndexBuffer()
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

// TODO - use addBuffer
bool InstancedVertexVulkanShaderPipeline::initInstanceBuffer()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const InstancedVertexShaderPipelineUpdateData &instancedVertexUpdateData =
        dynamic_cast<const InstancedVertexShaderPipelineUpdateData &>(
            *updateData_
        );

    vk::DeviceSize bufferSize =
        sizeof(Instance) * instancedVertexUpdateData.instances->size();
    auto [stagingBuffer, stagingBufferMemory] = renderer->createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );

    void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(
        dataStaging, instancedVertexUpdateData.instances->data(), bufferSize
    );
    stagingBufferMemory.unmapMemory();

    std::tie(instanceBuffer_, instanceBufferMemory_) = renderer->createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    renderer->copyBuffer(stagingBuffer, instanceBuffer_, bufferSize);

    return true;
}

bool InstancedVertexVulkanShaderPipeline::bind()
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
    commandBuffer.bindVertexBuffers(1, *instanceBuffer_, {0});
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

bool InstancedVertexVulkanShaderPipeline::update()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    const InstancedVertexShaderPipelineUpdateData &instancedVertexUpdateData =
        dynamic_cast<const InstancedVertexShaderPipelineUpdateData &>(
            *updateData_
        );

    // MVP uniform buffer (index 0)
    MVPBuffer mvp{};
    mvp.model =
        *instancedVertexUpdateData.actor->getComponent<TransformComponent>()
             ->getTransformMatrixRef();
    mvp.view = *instancedVertexUpdateData.camera->getViewMatrixRef();
    mvp.projection =
        *instancedVertexUpdateData.camera->getProjectionMatrixRef();
    memcpy(
        uniformBuffers_[0][renderer->getCurrentFrameIndex()].getMappedPointer(),
        &mvp, sizeof(mvp)
    );

    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {DebugDataBuffer()};
    storageRWBuffers_.front()[renderer->getCurrentFrameIndex()].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    return true;
}

bool InstancedVertexVulkanShaderPipeline::draw()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const InstancedVertexShaderPipelineUpdateData &instancedVertexUpdateData =
        dynamic_cast<const InstancedVertexShaderPipelineUpdateData &>(
            *updateData_
        );

    renderer->getCommandBuffer().drawIndexed(
        static_cast<uint32_t>(geometry_->indices.size()),
        instancedVertexUpdateData.instances->size(), 0, 0, 0
    );

    return true;
}

bool InstancedVertexVulkanShaderPipeline::postRender()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer->getCurrentFrameIndex()].copyToStaging(
        debugDataBufferSize, vk::PipelineStageFlagBits::eVertexShader |
                                 vk::PipelineStageFlagBits::eFragmentShader
    );
    return true;
}

PipelineStatistics InstancedVertexVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedVertexVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return false;
    isDebugQuery_ = false;

    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    // TODO - this is a bit extreme
    renderer->waitAllFramesComplete();

    pipelineStatistics_.pipelineType =
        PipelineStatistics::PipelineType::TRADITIONAL;

    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer->getCurrentFrameIndex()].readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nVertexShaderInvocations =
        debugDataBufferData_[0].nVertexShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    return true;
}
