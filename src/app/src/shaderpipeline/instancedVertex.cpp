#include "shaderpipeline/instancedVertex.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedVertexVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<InstancedVertexShaderPipelineUpdateData *>(updateData);
    geometry_ = geometry;
    return initPipeline();
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool InstancedVertexVulkanShaderPipeline::initPipeline()
{
    initBuffers();
    if (!initDescriptorLayout())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
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
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexVulkanShaderPipeline::initBufferDescriptions()
{
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::MVP)
            .setBufferSize(sizeof(InstancedVF::MVPBuffer))
            .setData(static_cast<InstancedVF::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedVF::AnimationBuffer))
            .setData(static_cast<InstancedVF::AnimationBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
    );
    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(InstancedVF::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedVF::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        ImageDescription()
            .setBufferSlot(InstancedVF::BufferSlots::TEXTURE)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setExtent(
                vk::Extent3D(
                    static_cast<uint32_t>(updateData_->texture->texWidth),
                    static_cast<uint32_t>(updateData_->texture->texHeight), 1
                )
            )
            .setStageFlags(vk::ShaderStageFlagBits::eFragment)
            .setData(updateData_->texture->pixels)
            .setDataSize(updateData_->texture->imageSize)
    );
}

void InstancedVertexVulkanShaderPipeline::initBuffers()
{
    initBufferDescriptions();

    for (auto bufferDescription : resourceDescriptions_)
        addResource(bufferDescription);

    BufferDescription vertexDesc = VertexBuffer;
    vertexDesc.bufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    vertexDesc.data = geometry_->vertices.data();
    vertexBuffer_.init(vertexDesc);

    BufferDescription indexDesc = IndexBuffer;
    indexDesc.bufferSize = sizeof(uint32_t) * geometry_->indices.size();
    indexDesc.data = geometry_->indices.data();
    indexBuffer_.init(indexDesc);

    BufferDescription instanceDesc = VertexBuffer;
    instanceDesc.bufferSize = sizeof(Instance) * MAX_N_INSTANCE_COLS *
                              MAX_N_INSTANCE_LAYERS * MAX_N_INSTANCE_ROWS;
    instanceDesc.data = nullptr;
    instanceBuffer_.init(instanceDesc);
}

bool InstancedVertexVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFVertex")
    );
    VulkanShader *instancedFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFFragment")
    );

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(instancedVertexShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(instancedFragmentShader->getShaderModule())
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
        renderer_->getInputAssembly();

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
        renderer_->getRasteriser();

    vk::PipelineMultisampleStateCreateInfo multisampling =
        renderer_->getMultisampling();

    vk::PipelineColorBlendAttachmentState colourBlendAttachment =
        renderer_->getColourBlendAttachment();

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

    std::vector<vk::PushConstantRange> pushConstantRanges = {
        vk::PushConstantRange{}
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
            .setOffset(0)
            .setSize(sizeof(InstancedVF::Constants)),
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount(1)
            .setPSetLayouts(&*descriptorSetLayout_)
            .setPushConstantRangeCount(pushConstantRanges.size())
            .setPPushConstantRanges(pushConstantRanges.data());

    pipelineLayout_ =
        vk::raii::PipelineLayout(renderer_->getDevice(), pipelineLayoutInfo);

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
        renderer_->getPipelineRendereringCreateInfo();

    vk::StructureChain<
        vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo>
        pipelineCreateInfoChain = {
            graphicsPipelineCreateInfo, pipelineRenderingCreateInfo
        };

    graphicsPipeline_ = vk::raii::Pipeline(
        renderer_->getDevice(), nullptr,
        pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()
    );
    return true;
}

bool InstancedVertexVulkanShaderPipeline::bind()
{
    spdlog::debug("Binding shader pipeline");

    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::Extent2D &swapChainExtent = renderer_->getSwapChainExtent();

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *getGraphicsPipeline()
    );
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    commandBuffer.bindVertexBuffers(
        0, **vertexBuffer_.getBuffer(frameIndex), {0}
    );
    commandBuffer.bindIndexBuffer(
        **indexBuffer_.getBuffer(frameIndex), 0, vk::IndexType::eUint32
    );
    commandBuffer.bindVertexBuffers(
        1, **instanceBuffer_.getBuffer(frameIndex), {0}
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipelineLayout_, 0,
        *descriptorSets_[renderer_->getCurrentFrameIndex()], nullptr
    );
    commandBuffer.setViewport(0, renderer_->getViewport());
    commandBuffer.setScissor(
        0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent)
    );
    spdlog::debug("Finished binding shader pipeline");

    return true;
}

bool InstancedVertexVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();

    // MVP uniform buffer (slot 0)
    InstancedVF::MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    resources_[InstancedVF::BufferSlots::MVP].update(sizeof(mvp), &mvp);

    InstancedVF::AnimationBuffer animation{};
    animation.isAnimationOn = *updateData_->isAnimationOn;
    animation.rotationFrequency = {
        *updateData_->rotationFrequency, 0.f, 0.f, 0.f
    };
    animation.deltaTime = {
        updateData_->timer->getDeltaSeconds(), 0.f, 0.f, 0.f
    };
    animation.currentTime = {
        updateData_->timer->getTotalSecondsElapsed(), 0.f, 0.f, 0.f
    };
    resources_[InstancedVF::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );

    vk::DeviceSize instanceBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    instanceBuffer_.update(instanceBufferSize, updateData_->instances->data());

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedVF::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    resources_[InstancedVF::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    InstancedVF::Constants constants = InstancedVF::Constants();
    constants.fragmentColourType = *updateData_->fragmentColourType;
    renderer_->getCommandBuffer().pushConstants(
        pipelineLayout_, vk::ShaderStageFlagBits::eVertex, 0,
        sizeof(InstancedVF::Constants), &constants
    );

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool InstancedVertexVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    uint32_t nIndices = geometry_->indexCounts[*updateData_->lodIndex];
    uint32_t indexOffset = geometry_->indexOffsets[*updateData_->lodIndex];
    renderer_->getCommandBuffer().drawIndexed(
        static_cast<uint32_t>(nIndices), updateData_->instances->size(),
        static_cast<uint32_t>(indexOffset), 0, 0
    );

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    nDrawCalls_++;

    return true;
}

bool InstancedVertexVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[InstancedVF::BufferSlots::DEBUG].asBuffer()->copyToStaging(
        debugDataBufferSize,
        vk::PipelineStageFlagBits::eVertexShader |
            vk::PipelineStageFlagBits::eFragmentShader,
        renderer_->getCurrentFrameIndex()
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

    pipelineStatistics_.pipelineType =
        PipelineStatistics::PipelineType::TRADITIONAL;

    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[InstancedVF::BufferSlots::DEBUG].asBuffer()->readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nVertexShaderInvocations =
        debugDataBufferData_[0].nVertexShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    pipelineStatistics_.nTriangles =
        (geometry_->meshletLODCounts[*updateData_->lodIndex] / 3.0) *
        updateData_->instances->size();
    pipelineStatistics_.nDrawCalls = nDrawCalls_;
    return true;
}
