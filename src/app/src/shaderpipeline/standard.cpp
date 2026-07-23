#include "shaderpipeline/standard.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"

bool StandardVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ = dynamic_cast<DefaultShaderPipelineUpdateData *>(updateData);
    geometry_ = geometry;
    return true;
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool StandardVulkanShaderPipeline::initPipeline()
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

bool StandardVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    renderer_->waitAllFramesComplete();
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool StandardVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    descriptorSetLayout_ = nullptr;
    descriptorSets_ = nullptr;
    descriptorPool_ = nullptr;
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for default vulkan pipeline");
    return true;
}

// TODO - change to resource, not buffer
void StandardVulkanShaderPipeline::initBufferDescriptions()
{
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(SimpleVF::BufferSlots::MVP)
            .setBufferSize(sizeof(SimpleVF::MVPBuffer))
            .setData(static_cast<SimpleVF::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );

    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(SimpleVF::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(SimpleVF::AnimationBuffer))
            .setData(static_cast<SimpleVF::AnimationBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
    );

    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(SimpleVF::BufferSlots::DEBUG)
            .setBufferSize(sizeof(SimpleVF::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );

    resourceDescriptions_.push_back(
        ImageDescription()
            .setBufferSlot(SimpleVF::BufferSlots::TEXTURE)
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

// TODO - rename buffer -> resource
void StandardVulkanShaderPipeline::initBuffers()
{
    initBufferDescriptions();

    for (auto resourceDescription : resourceDescriptions_)
        addResource(resourceDescription);

    BufferDescription vertexDesc = VertexBuffer;
    vertexDesc.bufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    vertexDesc.data = geometry_->vertices.data();
    vertexBuffer_.init(vertexDesc);

    BufferDescription indexDesc = IndexBuffer;
    indexDesc.bufferSize = sizeof(uint32_t) * geometry_->indices.size();
    indexDesc.data = geometry_->indices.data();
    indexBuffer_.init(indexDesc);
}

bool StandardVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *simpleVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("simpleVFVertex")
    );
    VulkanShader *simpleFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("simpleVFFragment")
    );

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eVertex)
            .setModule(simpleVertexShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(simpleFragmentShader->getShaderModule())
            .setPName("main");

    vk::VertexInputBindingDescription bindingDescription = {
        0, sizeof(VertexInputType), vk::VertexInputRate::eVertex
    };
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {
        {{0, 0, vk::Format::eR32G32B32Sfloat,
          offsetof(VertexInputType, position)},
         {1, 0, vk::Format::eR32G32Sfloat, offsetof(VertexInputType, tex)},
         {2, 0, vk::Format::eR32G32B32Sfloat,
          offsetof(VertexInputType, normal)}}
    };
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo =
        vk::PipelineVertexInputStateCreateInfo{}
            .setVertexBindingDescriptionCount(1)
            .setPVertexBindingDescriptions(&bindingDescription)
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

    vk::PipelineDepthStencilStateCreateInfo depthStencil =
        vk::PipelineDepthStencilStateCreateInfo{}
            .setDepthTestEnable(vk::True)
            .setDepthWriteEnable(vk::True)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthBoundsTestEnable(vk::False)
            .setStencilTestEnable(vk::False);

    std::vector<vk::PushConstantRange> pushConstantRanges = {
        vk::PushConstantRange{}
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
            .setOffset(0)
            .setSize(sizeof(SimpleVF::Constants)),
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

bool StandardVulkanShaderPipeline::bind()
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

bool StandardVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();

    // MVP uniform buffer (slot 0)
    SimpleVF::MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    resources_[SimpleVF::BufferSlots::MVP].update(sizeof(mvp), &mvp);

    SimpleVF::AnimationBuffer animation{};
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
    resources_[SimpleVF::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize = sizeof(SimpleVF::DebugDataBuffer);
    debugDataBufferData_ = {SimpleVF::DebugDataBuffer()};
    debugDataBufferData_.front().isDebugQueryOn = isDebugQuery_;
    resources_[SimpleVF::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    SimpleVF::Constants constants = SimpleVF::Constants();
    constants.fragmentColourType = *updateData_->fragmentColourType;
    constants.lodLevel = *updateData_->lodIndex;
    renderer_->getCommandBuffer().pushConstants(
        pipelineLayout_,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, sizeof(SimpleVF::Constants), &constants
    );

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool StandardVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    uint32_t nIndices = geometry_->indexCounts[*updateData_->lodIndex];
    uint32_t indexOffset = geometry_->indexOffsets[*updateData_->lodIndex];
    renderer_->getCommandBuffer().drawIndexed(
        static_cast<uint32_t>(nIndices), 1, static_cast<uint32_t>(indexOffset),
        0, 0
    );
    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    nDrawCalls_++;

    return true;
}

bool StandardVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(SimpleVF::DebugDataBuffer) * debugDataBufferData_.size();

    if (auto *buffer = resources_[SimpleVF::BufferSlots::DEBUG].asBuffer())
    {
        buffer->copyToStaging(
            debugDataBufferSize,
            vk::PipelineStageFlagBits::eVertexShader |
                vk::PipelineStageFlagBits::eFragmentShader,
            renderer_->getCurrentFrameIndex()
        );
    }

    return true;
}

PipelineStatistics StandardVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool StandardVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return false;

    vk::DeviceSize debugDataBufferSize =
        sizeof(SimpleVF::DebugDataBuffer) * debugDataBufferData_.size();

    if (auto *buffer = resources_[SimpleVF::BufferSlots::DEBUG].asBuffer())
    {
        buffer->readData(debugDataBufferSize, debugDataBufferData_.data());
        pipelineStatistics_.stats = {
            {
                "N Vertices",
                geometry_->vertices.size(),
            },
            {
                "N Triangles",
                geometry_->vertices.size() / 3,
            },
            {"N VS Invocations",
             debugDataBufferData_[0].nVertexShaderInvocations},
            {"N FS Invocations",
             debugDataBufferData_[0].nFragmentShaderInvocations},
            {"N Triangles",
             (long) (geometry_->meshletLODCounts[*updateData_->lodIndex] / 3)},
            {"N Draw Calls", nDrawCalls_},
        };
    }
    return true;
}
