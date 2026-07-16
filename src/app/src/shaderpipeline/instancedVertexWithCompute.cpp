#include "shaderpipeline/instancedVertexWithCompute.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedVertexComputeVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<InstancedVertexComputeShaderPipelineUpdateData *>(
            updateData
        );
    geometry_ = geometry;
    return initPipeline();
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool InstancedVertexComputeVulkanShaderPipeline::initPipeline()
{
    initBuffers();
    if (!initDescriptorLayout())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
        return false;
    if (!initComputeShaderPipeline())
        return false;
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    computePasses_.clear();
    graphicsPipeline_ = nullptr;
    if (!initGraphicsPipeline())
        return false;
    if (!initComputeShaderPipeline())
        return false;
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down instanced vulkan pipeline");
    pipelineLayout_ = nullptr;
    computePasses_.clear();
    shutdownBuffers();
    graphicsPipeline_ = nullptr;
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexComputeVulkanShaderPipeline::initBufferDescriptions()
{
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVFCompute::BufferSlots::MVP)
            .setBufferSize(sizeof(InstancedVFCompute::MVPBuffer))
            .setData(static_cast<InstancedVFCompute::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );

    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVFCompute::BufferSlots::SCENE)
            .setBufferSize(sizeof(InstancedVFCompute::SceneBuffer))
            .setData(static_cast<InstancedVFCompute::SceneBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVFCompute::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedVFCompute::AnimationBuffer))
            .setData(
                static_cast<InstancedVFCompute::AnimationBuffer *>(nullptr)
            )
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute
            )
    );

    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(InstancedVFCompute::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedVFCompute::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment |
                vk::ShaderStageFlagBits::eCompute
            )
    );

    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedVFCompute::BufferSlots::REORDERED_INSTANCES)
            .setBufferSize(
                sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
                MAX_N_INSTANCE_ROWS
            )
            .setData(nullptr)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedVFCompute::BufferSlots::LOD_STATS)
            .setBufferSize(sizeof(InstancedVFCompute::LODStats) * MAX_LOD_COUNT)
            .setData(nullptr)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(InstancedVFCompute::BufferSlots::INSTANCES)
            .setBufferSize(
                sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
                MAX_N_INSTANCE_ROWS
            )
            .setData(nullptr)
            .setStageFlags(
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eVertex
            )
    );

    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS)
            .setBufferSize(
                sizeof(vk::DrawIndexedIndirectCommand) * MAX_LOD_COUNT
            )
            .setDeviceUsage(
                StorageReadWrite.deviceUsage |
                vk::BufferUsageFlagBits::eIndirectBuffer
            )
            .setData(nullptr)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    resourceDescriptions_.push_back(
        ImageDescription()
            .setBufferSlot(InstancedVFCompute::BufferSlots::TEXTURE)
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

void InstancedVertexComputeVulkanShaderPipeline::initBuffers()
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

bool InstancedVertexComputeVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCVertex")
    );
    VulkanShader *instancedFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCFragment")
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
    std::array<vk::VertexInputAttributeDescription, 7> attributeDescriptions = {
        // vertex
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(0)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(VertexInputType, position)),
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(1)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(VertexInputType, tex)),
        vk::VertexInputAttributeDescription{}
            .setBinding(0)
            .setLocation(2)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
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

        // other
        vk::VertexInputAttributeDescription{}
            .setBinding(1)
            .setLocation(6)
            .setFormat(vk::Format::eR32G32B32A32Sfloat)
            .setOffset(offsetof(Instance, lodLevel)),

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
            .setStageFlags(vk::ShaderStageFlagBits::eFragment)
            .setOffset(0)
            .setSize(sizeof(InstancedVFCompute::Constants)),
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

bool InstancedVertexComputeVulkanShaderPipeline::initComputeShaderPipeline()
{
    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();

    VulkanShader *lodStatsComputeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCLODStats")
    );
    VulkanShader *drawCommandComputeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCDrawCommand")
    );

    {
        vk::PipelineShaderStageCreateInfo lodStatsStageInfo =
            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setModule(lodStatsComputeShader->getShaderModule())
                .setPName("main");

        vk::ComputePipelineCreateInfo computePipelineCreateInfo =
            vk::ComputePipelineCreateInfo{}
                .setStage(lodStatsStageInfo)
                .setLayout(pipelineLayout_);

        computePasses_.emplace_back(
            vk::raii::Pipeline(
                renderer_->getDevice(), nullptr, computePipelineCreateInfo
            )
        );
    }

    {
        vk::PipelineShaderStageCreateInfo drawCommandStageInfo =
            vk::PipelineShaderStageCreateInfo{}
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setModule(drawCommandComputeShader->getShaderModule())
                .setPName("main");

        vk::ComputePipelineCreateInfo computePipelineCreateInfo =
            vk::ComputePipelineCreateInfo{}
                .setStage(drawCommandStageInfo)
                .setLayout(pipelineLayout_);

        computePasses_.emplace_back(
            vk::raii::Pipeline(
                renderer_->getDevice(), nullptr, computePipelineCreateInfo
            )
        );
    }
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::bind()
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

void InstancedVertexComputeVulkanShaderPipeline::updateSceneData()
{
    InstancedVFCompute::SceneBuffer sceneData{};
    sceneData.eyePosition = {
        updateData_->camera->getComponent<TransformComponent>()->getPosition(),
        1.0f
    };
    sceneData.instanceCount = {
        static_cast<uint32_t>(updateData_->instances->size()), 0, 0, 0
    };
    sceneData.minMeshBounds = {geometry_->meshBounds.minCorner, 1.0f};
    sceneData.maxMeshBounds = {geometry_->meshBounds.maxCorner, 1.0f};
    sceneData.maxLODDistance = {*updateData_->maxLODDistance, 0.f, 0.f, 0.f};
    sceneData.lodExponent = {*updateData_->lodExponent, 0.f, 0.f, 0.f};

    std::array<FrustumPlane, 6> frustumPlanes =
        updateData_->camera->getFrustumPlanes();
    FrustumCone frustumCone = updateData_->camera->getFrustumCone(true);

    for (int planeIndex = 0; planeIndex < 6; planeIndex++)
    {
        sceneData.frustum.planes[planeIndex] = frustumPlanes[planeIndex];
    }

    sceneData.frustum.sphere = updateData_->camera->getFrustumSphere();
    sceneData.frustum.cone.tip = frustumCone.tip;
    sceneData.frustum.cone.height = frustumCone.height;
    sceneData.frustum.cone.direction = frustumCone.direction;
    sceneData.frustum.cone.angle = frustumCone.angle;
    sceneData.visibilityFunc = {
        static_cast<uint32_t>(*updateData_->visibilityFunc), 0, 0, 0
    };

    resources_[InstancedVFCompute::BufferSlots::SCENE].update(
        sizeof(sceneData), &sceneData
    );
}

void InstancedVertexComputeVulkanShaderPipeline::updateMVPData()
{
    InstancedVFCompute::MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    resources_[InstancedVFCompute::BufferSlots::MVP].update(sizeof(mvp), &mvp);
}

void InstancedVertexComputeVulkanShaderPipeline::updateAnimationData()
{
    InstancedVFCompute::AnimationBuffer animation{};
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
    resources_[InstancedVFCompute::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );
}

void InstancedVertexComputeVulkanShaderPipeline::updateInstancesData()
{
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    resources_[InstancedVFCompute::BufferSlots::INSTANCES].update(
        instancesBufferSize, updateData_->instances->data()
    );
}

void InstancedVertexComputeVulkanShaderPipeline::updateDebugData()
{
    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedVFCompute::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    resources_[InstancedVFCompute::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );
}

void InstancedVertexComputeVulkanShaderPipeline::resetIndirectDrawCommands()
{
    std::vector<vk::DrawIndexedIndirectCommand> indirectCommandsData(
        MAX_LOD_COUNT, vk::DrawIndexedIndirectCommand()
    );
    for (unsigned int lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        indirectCommandsData[lodIndex].indexCount =
            geometry_->indexCounts[lodIndex];
        indirectCommandsData[lodIndex].instanceCount = 0;
        indirectCommandsData[lodIndex].firstIndex =
            geometry_->indexOffsets[lodIndex];
        indirectCommandsData[lodIndex].vertexOffset = 0;
        indirectCommandsData[lodIndex].firstInstance = 0;
    }
    resources_[InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS].update(
        sizeof(vk::DrawIndexedIndirectCommand) * MAX_LOD_COUNT,
        indirectCommandsData.data()
    );
}

void InstancedVertexComputeVulkanShaderPipeline::resetLODStats()
{
    std::vector<InstancedVFCompute::LODStats> lodStats(
        MAX_LOD_COUNT, InstancedVFCompute::LODStats()
    );
    resources_[InstancedVFCompute::BufferSlots::LOD_STATS].update(
        sizeof(InstancedVFCompute::LODStats) * MAX_LOD_COUNT, lodStats.data()
    );
}

void InstancedVertexComputeVulkanShaderPipeline::bindComputePipeline(uint pass)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute, *computePasses_[pass]
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, pipelineLayout_, 0,
        *descriptorSets_[renderer_->getCurrentFrameIndex()], nullptr
    );
}

void InstancedVertexComputeVulkanShaderPipeline::lodStatsComputePass(
    uint dispatchCount
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

    commandBuffer.dispatch(dispatchCount, 1, 1);

    MemoryBarrierInfo memoryBarrierInfo = {
        .sourceAccess = vk::AccessFlagBits2::eShaderRead,
        .destinationAccess = vk::AccessFlagBits2::eShaderRead |
                             vk::AccessFlagBits2::eShaderWrite,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eComputeShader,
    };
    renderer_->createPipelineMemoryBarrier(memoryBarrierInfo);
}

void InstancedVertexComputeVulkanShaderPipeline::drawCommandComputePass(
    uint dispatchCount
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

    commandBuffer.dispatch(dispatchCount, 1, 1);

    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    VulkanBuffer *reorderedInstancesBuffer =
        resources_[InstancedVFCompute::BufferSlots::REORDERED_INSTANCES]
            .asBuffer();
    VulkanBuffer *indirectBuffer =
        resources_[InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS]
            .asBuffer();

    BufferBarrierInfo barrierInfo;

    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eTransferRead,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eTransfer,
        .buffer = reorderedInstancesBuffer->getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);

    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eHostWrite,
        .destinationAccess = vk::AccessFlagBits2::eTransferWrite,
        .sourceStage = vk::PipelineStageFlagBits2::eHost,
        .destinationStage = vk::PipelineStageFlagBits2::eTransfer,
        .buffer = instanceBuffer_.getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);

    setBarrier();

    vk::DeviceSize maxInstancesSize = sizeof(Instance) * MAX_N_INSTANCE_COLS *
                                      MAX_N_INSTANCE_LAYERS *
                                      MAX_N_INSTANCE_ROWS;
    commandBuffer.copyBuffer(
        *reorderedInstancesBuffer->getBuffer(frameIndex),
        *instanceBuffer_.getBuffer(frameIndex),
        vk::BufferCopy(0, 0, maxInstancesSize)
    );

    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eIndirectCommandRead,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eDrawIndirect,
        .buffer = indirectBuffer->getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);
    setBarrier();
}

bool InstancedVertexComputeVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    updateSceneData();
    updateMVPData();
    updateAnimationData();
    updateInstancesData();
    updateDebugData();

    InstancedVFCompute::Constants constants = InstancedVFCompute::Constants();
    constants.fragmentColourType = *updateData_->fragmentColourType;
    renderer_->getCommandBuffer().pushConstants(
        pipelineLayout_, vk::ShaderStageFlagBits::eFragment, 0,
        sizeof(InstancedVFCompute::Constants), &constants
    );

    resetIndirectDrawCommands();
    resetLODStats();

    uint32_t dispatchCount =
        (updateData_->instances->size() + THREAD_GROUP_SIZE - 1) /
        THREAD_GROUP_SIZE;
    bindComputePipeline(0);
    lodStatsComputePass(dispatchCount);
    bindComputePipeline(1);
    drawCommandComputePass(dispatchCount);

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    renderer_->getCommandBuffer().drawIndexedIndirect(
        *resources_[InstancedVFCompute::BufferSlots::INDIRECT_COMMANDS]
             .asBuffer()
             ->getBuffer(frameIndex),
        0, MAX_LOD_COUNT, sizeof(vk::DrawIndexedIndirectCommand)
    );
    nDrawCalls_++;

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    resources_[InstancedVFCompute::BufferSlots::DEBUG]
        .asBuffer()
        ->copyToStaging(
            debugDataBufferSize,
            vk::PipelineStageFlagBits::eVertexShader |
                vk::PipelineStageFlagBits::eFragmentShader,
            renderer_->getCurrentFrameIndex()
        );
    return true;
}

PipelineStatistics
InstancedVertexComputeVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedVertexComputeVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return false;

    pipelineStatistics_.pipelineType =
        PipelineStatistics::PipelineType::TRADITIONAL;

    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    resources_[InstancedVFCompute::BufferSlots::DEBUG].asBuffer()->readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nVertexShaderInvocations =
        debugDataBufferData_[0].nVertexShaderInvocations;
    pipelineStatistics_.nComputeShaderInvocations =
        debugDataBufferData_[0].nComputeShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    pipelineStatistics_.nTriangles =
        (geometry_->meshletLODCounts[*updateData_->lodIndex] / 3.0) *
        updateData_->instances->size();
    pipelineStatistics_.nDrawCalls = nDrawCalls_;
    return true;
}
