#include "shaderpipeline/instancedVertexWithComputeFlat.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedVertexComputeFlatVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<InstancedVertexComputeFlatShaderPipelineUpdateData *>(
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
bool InstancedVertexComputeFlatVulkanShaderPipeline::initPipeline()
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

bool InstancedVertexComputeFlatVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down instanced vulkan pipeline");
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    computePipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexComputeFlatVulkanShaderPipeline::initBufferDescriptions()
{
    bufferDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVFComputeFlat::BufferSlots::MVP)
            .setBufferSize(sizeof(InstancedVFComputeFlat::MVPBuffer))
            .setData(static_cast<InstancedVFComputeFlat::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );

    bufferDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVFComputeFlat::BufferSlots::SCENE)
            .setBufferSize(sizeof(InstancedVFComputeFlat::SceneBuffer))
            .setData(
                static_cast<InstancedVFComputeFlat::SceneBuffer *>(nullptr)
            )
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    bufferDescriptions_.push_back(
        UniformBuffer
            .setBufferSlot(InstancedVFComputeFlat::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedVFComputeFlat::AnimationBuffer))
            .setData(
                static_cast<InstancedVFComputeFlat::AnimationBuffer *>(nullptr)
            )
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute
            )
    );

    bufferDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedVFComputeFlat::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedVFComputeFlat::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment |
                vk::ShaderStageFlagBits::eCompute
            )
    );

    bufferDescriptions_.push_back(
        StorageRead
            .setBufferSlot(InstancedVFComputeFlat::BufferSlots::INSTANCES)
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

    bufferDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(
                InstancedVFComputeFlat::BufferSlots::INDIRECT_COMMANDS
            )
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

    bufferDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(
                InstancedVFComputeFlat::BufferSlots::INDIRECT_DRAW_STATS
            )
            .setBufferSize(
                sizeof(InstancedVFComputeFlat::IndirectDrawStats) *
                MAX_LOD_COUNT
            )
            .setDeviceUsage(
                StorageReadWrite.deviceUsage |
                vk::BufferUsageFlagBits::eIndirectBuffer
            )
            .setData(nullptr)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );

    bufferDescriptions_.push_back(
        StorageRead.setBufferSlot(InstancedVFComputeFlat::BufferSlots::LOD)
            .setBufferSize(sizeof(InstancedVFComputeFlat::LOD) * MAX_LOD_COUNT)
            .setData(nullptr)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::initBuffers()
{
    initBufferDescriptions();

    for (auto bufferDescription : bufferDescriptions_)
        addBuffer(bufferDescription);

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

bool InstancedVertexComputeFlatVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCFlatVertex")
    );
    VulkanShader *instancedFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCFlatFragment")
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

    vk::PushConstantRange pushConstantRange =
        vk::PushConstantRange{}
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
            .setOffset(0)
            .setSize(sizeof(uint32_t));

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount(1)
            .setPSetLayouts(&*descriptorSetLayout_)
            .setPushConstantRangeCount(1)
            .setPPushConstantRanges(&pushConstantRange);

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

bool InstancedVertexComputeFlatVulkanShaderPipeline::initComputeShaderPipeline()
{
    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *computeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFCFlatCompute")
    );
    vk::PipelineShaderStageCreateInfo computeShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(computeShader->getShaderModule())
            .setPName("main");

    vk::ComputePipelineCreateInfo computePipelineCreateInfo =
        vk::ComputePipelineCreateInfo{}
            .setStage(computeShaderStageInfo)
            .setLayout(pipelineLayout_);

    computePipeline_ = vk::raii::Pipeline(
        renderer_->getDevice(), nullptr, computePipelineCreateInfo
    );
    return true;
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::bind()
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

void InstancedVertexComputeFlatVulkanShaderPipeline::updateSceneData()
{
    InstancedVFComputeFlat::SceneBuffer sceneData{};
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

    buffers_[InstancedVFComputeFlat::BufferSlots::SCENE].update(
        sizeof(sceneData), &sceneData
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::updateMVPData()
{
    InstancedVFComputeFlat::MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    buffers_[InstancedVFComputeFlat::BufferSlots::MVP].update(
        sizeof(mvp), &mvp
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::updateAnimationData()
{
    InstancedVFComputeFlat::AnimationBuffer animation{};
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
    buffers_[InstancedVFComputeFlat::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::updateInstancesData()
{
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    buffers_[InstancedVFComputeFlat::BufferSlots::INSTANCES].update(
        instancesBufferSize, updateData_->instances->data()
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::updateDebugData()
{
    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFComputeFlat::DebugDataBuffer) *
        debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedVFComputeFlat::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    buffers_[InstancedVFComputeFlat::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::resetIndirectDrawCommands()
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
    buffers_[InstancedVFComputeFlat::BufferSlots::INDIRECT_COMMANDS].update(
        sizeof(vk::DrawIndexedIndirectCommand) * MAX_LOD_COUNT,
        indirectCommandsData.data()
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::resetIndirectDrawStats()
{
    std::vector<InstancedVFComputeFlat::IndirectDrawStats>
        indirectDrawStatsData(
            MAX_LOD_COUNT, InstancedVFComputeFlat::IndirectDrawStats()
        );
    buffers_[InstancedVFComputeFlat::BufferSlots::INDIRECT_DRAW_STATS].update(
        sizeof(InstancedVFComputeFlat::IndirectDrawStats) * MAX_LOD_COUNT,
        indirectDrawStatsData.data()
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::bindComputePipeline()
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute, *computePipeline_
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, pipelineLayout_, 0,
        *descriptorSets_[renderer_->getCurrentFrameIndex()], nullptr
    );
}

void InstancedVertexComputeFlatVulkanShaderPipeline::computePass()
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();

    // dispatch
    uint32_t dispatchCount = (updateData_->instances->size() + 255) / 256;
    commandBuffer.dispatch(dispatchCount, 1, 1);

    VulkanBuffer &indirectBuffer =
        buffers_[InstancedVFComputeFlat::BufferSlots::INDIRECT_COMMANDS];
    vk::BufferMemoryBarrier2 bufferBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eIndirectCommandRead)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
            .setDstStageMask(vk::PipelineStageFlagBits2::eDrawIndirect)
            .setBuffer(*indirectBuffer.getBuffer(frameIndex))
            .setSize(VK_WHOLE_SIZE);
    vk::DependencyInfo dependencyInfo =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&bufferBarrier);
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    updateSceneData();
    updateMVPData();
    updateAnimationData();
    updateInstancesData();
    updateDebugData();

    resetIndirectDrawCommands();
    resetIndirectDrawStats();

    bindComputePipeline();
    computePass();

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    for (unsigned int lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        renderer_->getCommandBuffer().drawIndexedIndirect(
            *buffers_[InstancedVFComputeFlat::BufferSlots::INDIRECT_COMMANDS]
                 .getBuffer(frameIndex),
            lodIndex * sizeof(vk::DrawIndexedIndirectCommand), 1,
            sizeof(vk::DrawIndexedIndirectCommand)
        );
        nDrawCalls_++;
    }

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFComputeFlat::DebugDataBuffer) *
        debugDataBufferData_.size();
    buffers_[InstancedVFComputeFlat::BufferSlots::DEBUG].copyToStaging(
        debugDataBufferSize,
        vk::PipelineStageFlagBits::eVertexShader |
            vk::PipelineStageFlagBits::eFragmentShader,
        renderer_->getCurrentFrameIndex()
    );
    return true;
}

PipelineStatistics
InstancedVertexComputeFlatVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedVertexComputeFlatVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return false;

    pipelineStatistics_.pipelineType =
        PipelineStatistics::PipelineType::TRADITIONAL;

    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVFComputeFlat::DebugDataBuffer) *
        debugDataBufferData_.size();
    buffers_[InstancedVFComputeFlat::BufferSlots::DEBUG].readData(
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
