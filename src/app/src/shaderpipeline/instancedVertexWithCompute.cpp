#include "shaderpipeline/instancedVertexWithCompute.h"
#include "core_buffertypes/buffertypes.h"
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
    initUniformBuffers();
    initStorageBuffers();
    if (!initDescriptorLayout())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
        return false;
    if (!initComputeShaderPipeline())
        return false;
    if (!initVertexBuffer())
        return false;
    if (!initIndexBuffer())
        return false;
    if (!initInstanceBuffer())
        return false;
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedVertexComputeVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down instanced vulkan pipeline");
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    computePipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexComputeVulkanShaderPipeline::initUniformBuffers()
{
    addBuffer<VulkanBufferUniform>(
        uniformBuffers_, sizeof(MVPBuffer), static_cast<MVPBuffer *>(nullptr),
        vk::ShaderStageFlagBits::eVertex
    );

    addBuffer<VulkanBufferUniform>(
        uniformBuffers_, sizeof(SceneBuffer),
        static_cast<SceneBuffer *>(nullptr), vk::ShaderStageFlagBits::eCompute
    );

    addBuffer<VulkanBufferUniform>(
        uniformBuffers_, sizeof(AnimationBuffer),
        static_cast<AnimationBuffer *>(nullptr),
        vk::ShaderStageFlagBits::eVertex
    );
}

void InstancedVertexComputeVulkanShaderPipeline::initStorageBuffers()
{
    // Binding 3: debugData (RW Storage)
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    addBuffer<VulkanBufferRWStorage>(
        storageRWBuffers_, debugDataBufferSize, debugDataBufferData_.data(),
        vk::ShaderStageFlagBits::eAll
    );

    // Binding 4: reorderedInstances (RW Storage)
    vk::DeviceSize reorderedInstancesSize =
        sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
        MAX_N_INSTANCE_ROWS;
    addBuffer<VulkanBufferRWStorage>(
        storageRWBuffers_, reorderedInstancesSize, nullptr,
        vk::ShaderStageFlagBits::eCompute
    );

    // Binding 5: lodBaseOffsets (RW Storage)
    vk::DeviceSize lodBaseOffsetsSize = sizeof(uint32_t) * MAX_LOD_COUNT;
    addBuffer<VulkanBufferRWStorage>(
        storageRWBuffers_, lodBaseOffsetsSize, nullptr,
        vk::ShaderStageFlagBits::eCompute
    );

    // Binding 6: lodSlotCounters (RW Storage)
    vk::DeviceSize lodSlotCountersSize = sizeof(uint32_t) * MAX_LOD_COUNT;
    addBuffer<VulkanBufferRWStorage>(
        storageRWBuffers_, lodSlotCountersSize, nullptr,
        vk::ShaderStageFlagBits::eCompute
    );

    // Binding 7: instances (R Storage)
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
        MAX_N_INSTANCE_ROWS;
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, instancesBufferSize, nullptr,
        vk::ShaderStageFlagBits::eCompute
    );

    // Binding 8: indirectCommands (Indirect Storage)
    vk::DeviceSize indirectDrawBufferSize =
        sizeof(vk::DrawIndexedIndirectCommand) * MAX_LOD_COUNT;
    addBuffer<VulkanBufferIndirectStorage>(
        storageIndirectBuffers_, indirectDrawBufferSize, nullptr,
        vk::ShaderStageFlagBits::eAll
    );
}

bool InstancedVertexComputeVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedComputeVertex")
    );
    VulkanShader *defaultFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedFragment")
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

bool InstancedVertexComputeVulkanShaderPipeline::initComputeShaderPipeline()
{
    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *computeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedCompute")
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

bool InstancedVertexComputeVulkanShaderPipeline::initVertexBuffer()
{
    vk::DeviceSize bufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    vertexBuffer_.init(bufferSize, geometry_->vertices.data());
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::initIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(uint32_t) * geometry_->indices.size();
    indexBuffer_.init(bufferSize, geometry_->indices.data());
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::initInstanceBuffer()
{
    vk::DeviceSize bufferSize = sizeof(Instance) * MAX_N_INSTANCE_COLS *
                                MAX_N_INSTANCE_LAYERS * MAX_N_INSTANCE_ROWS;
    instanceBuffer_.init(bufferSize, nullptr);

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
    commandBuffer.bindVertexBuffers(0, **vertexBuffer_.getBuffer(), {0});
    commandBuffer.bindIndexBuffer(
        **indexBuffer_.getBuffer(), 0, vk::IndexType::eUint32
    );
    commandBuffer.bindVertexBuffers(1, **instanceBuffer_.getBuffer(), {0});
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

bool InstancedVertexComputeVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    SceneBuffer sceneData{};
    sceneData.eyePosition = {
        updateData_->camera->getComponent<TransformComponent>()->getPosition(),
        1.0f
    };
    sceneData.view = *updateData_->camera->getViewMatrixRef();
    sceneData.projection = *updateData_->camera->getProjectionMatrixRef();
    sceneData.instanceCount = updateData_->instances->size();
    sceneData.meshletCount = geometry_->meshletLODCounts[0];
    for (unsigned int lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        sceneData.indexCounts[lodIndex] = {
            geometry_->indexCounts[lodIndex], 0.f, 0.f, 0.f
        };
        sceneData.indexOffsets[lodIndex] = {
            geometry_->indexOffsets[lodIndex], 0.f, 0.f, 0.f
        };
        sceneData.vertexOffsets[lodIndex] = {
            geometry_->vertexOffsets[lodIndex], 0.f, 0.f, 0.f
        };
    }
    for (unsigned int lodIndex = 0;
         lodIndex < geometry_->meshletLODOffsets.size(); lodIndex++)
    {
        sceneData.meshletLODOffsets[lodIndex][0] =
            geometry_->meshletLODOffsets[lodIndex];
        sceneData.meshletLODCounts[lodIndex][0] =
            geometry_->meshletLODCounts[lodIndex];
    }
    sceneData.minMeshBounds = {geometry_->meshBounds.minCorner, 1.0f};
    sceneData.maxMeshBounds = {geometry_->meshBounds.maxCorner, 1.0f};
    sceneData.maxLODDistance = {*updateData_->maxLODDistance, 0.f, 0.f, 0.f};
    sceneData.lodExponent = {*updateData_->lodExponent, 0.f, 0.f, 0.f};
    sceneData.isLODOn = *updateData_->isLODOn;

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
    sceneData.visibilityFunc = *updateData_->visibilityFunc;

    uniformBuffers_[sceneDataUniformBufferIndex_]
                   [renderer_->getCurrentFrameIndex()]
                       .update(sizeof(sceneData), &sceneData);

    MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    uniformBuffers_[0][renderer_->getCurrentFrameIndex()].update(
        sizeof(mvp), &mvp
    );

    AnimationBuffer animation{};
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
    uniformBuffers_[animationDataUniformBufferIndex_]
                   [renderer_->getCurrentFrameIndex()]
                       .update(sizeof(animation), &animation);

    // This takes up most of the frame time
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    storageRBuffers_[instancesStorageRBufferIndex_]
                    [renderer_->getCurrentFrameIndex()]
                        .update(
                            instancesBufferSize, updateData_->instances->data()
                        );

    // instanceBuffer_ is populated by GPU copy from reorderedInstances

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {DebugDataBuffer()};
    debugDataBufferData_[debugDataStorageRWBufferIndex_].isDebugQueryOn =
        isDebugQuery_;
    storageRWBuffers_[debugDataStorageRWBufferIndex_]
                     [renderer_->getCurrentFrameIndex()]
                         .update(
                             debugDataBufferSize, debugDataBufferData_.data()
                         );

    // Dispatch compute shader outside of render pass
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute, *computePipeline_
    );
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute, pipelineLayout_, 0,
        *descriptorSets_[renderer_->getCurrentFrameIndex()], nullptr
    );
    uint32_t dispatchCount = (updateData_->instances->size() + 255) / 256;

    // Clear indirect commands buffer on CPU: populate geometry data, zero
    // counts This avoids a race condition where the reset thread could execute
    // after atomic adds from other workgroups.
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
        // vertexOffset is 0 because indices are already adjusted during
        // resource loading to point into the combined vertex buffer.
        // Setting this to geometry_->vertexOffsets[lodIndex] would
        // double-apply the offset and cause out-of-bounds vertex fetches.
        indirectCommandsData[lodIndex].vertexOffset = 0;
        indirectCommandsData[lodIndex].firstInstance = 0;
    }
    storageIndirectBuffers_.front()[renderer_->getCurrentFrameIndex()].update(
        sizeof(vk::DrawIndexedIndirectCommand) * MAX_LOD_COUNT,
        indirectCommandsData.data()
    );

    // Clear slot counters on CPU before dispatch. This avoids a race condition
    // where AllMemoryBarrierWithGroupSync() only synchronises within a
    // workgroup, leaving cross-workgroup resets and scatters unordered.
    std::vector<uint32_t> slotCountersData(MAX_LOD_COUNT, 0);
    storageRWBuffers_[lodSlotCountersStorageRWBufferIndex_]
                     [renderer_->getCurrentFrameIndex()]
                         .update(
                             sizeof(uint32_t) * MAX_LOD_COUNT,
                             slotCountersData.data()
                         );

    // Dispatch 1: Compute LOD per instance, atomic count
    uint32_t phase0 = 0;
    commandBuffer.pushConstants(
        pipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t),
        &phase0
    );
    commandBuffer.dispatch(dispatchCount, 1, 1);

    // Barrier: ensure compute writes from phase 0 are visible to phase 1
    // (indirectCommands.instanceCount, lodBaseOffsets, lodSlotCounters)
    vk::MemoryBarrier2 computeToComputeBarrier =
        vk::MemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
            .setDstAccessMask(
                vk::AccessFlagBits2::eShaderRead |
                vk::AccessFlagBits2::eShaderWrite
            )
            .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
            .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader);
    vk::DependencyInfo phaseBarrierInfo =
        vk::DependencyInfo{}.setMemoryBarrierCount(1).setPMemoryBarriers(
            &computeToComputeBarrier
        );
    commandBuffer.pipelineBarrier2(phaseBarrierInfo);

    // Dispatch 2: Compute base offsets, scatter instances, set firstInstance
    uint32_t phase1 = 1;
    commandBuffer.pushConstants(
        pipelineLayout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(uint32_t),
        &phase1
    );
    commandBuffer.dispatch(dispatchCount, 1, 1);

    // Barrier: ensure compute writes to reordered instances are visible for
    // copy
    auto &reorderedInstancesBuffer = storageRWBuffers_
        [1][renderer_->getCurrentFrameIndex()]; // reordered instances buffer
    vk::BufferMemoryBarrier2 reorderBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
            .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setBuffer(*reorderedInstancesBuffer.getBuffer())
            .setSize(VK_WHOLE_SIZE);
    vk::BufferMemoryBarrier2 instanceBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eHostWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eHost)
            .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setBuffer(*instanceBuffer_.getBuffer())
            .setSize(VK_WHOLE_SIZE);
    vk::DependencyInfo reorderDependencyInfo =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(2)
            .setPBufferMemoryBarriers(
                std::array<vk::BufferMemoryBarrier2, 2>{
                    reorderBarrier, instanceBarrier
                }
                    .data()
            );
    commandBuffer.pipelineBarrier2(reorderDependencyInfo);

    // Copy reordered instances to vertex input buffer (on same command buffer)
    vk::DeviceSize maxInstancesSize = sizeof(Instance) * MAX_N_INSTANCE_COLS *
                                      MAX_N_INSTANCE_LAYERS *
                                      MAX_N_INSTANCE_ROWS;
    commandBuffer.copyBuffer(
        *reorderedInstancesBuffer.getBuffer(), *instanceBuffer_.getBuffer(),
        vk::BufferCopy(0, 0, maxInstancesSize)
    );

    // Barrier: ensure reordered instances are visible for vertex fetch
    vk::BufferMemoryBarrier2 vertexFetchBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eVertexAttributeRead)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
            .setDstStageMask(vk::PipelineStageFlagBits2::eVertexInput)
            .setBuffer(*instanceBuffer_.getBuffer())
            .setSize(VK_WHOLE_SIZE);
    vk::DependencyInfo vertexFetchDependencyInfo =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&vertexFetchBarrier);
    commandBuffer.pipelineBarrier2(vertexFetchDependencyInfo);

    // Barrier: ensure compute writes to indirect buffer are visible for draw
    auto &indirectBuffer =
        storageIndirectBuffers_.front()[renderer_->getCurrentFrameIndex()];
    vk::BufferMemoryBarrier2 bufferBarrier =
        vk::BufferMemoryBarrier2{}
            .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
            .setDstAccessMask(vk::AccessFlagBits2::eIndirectCommandRead)
            .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
            .setDstStageMask(vk::PipelineStageFlagBits2::eDrawIndirect)
            .setBuffer(*indirectBuffer.getBuffer())
            .setSize(VK_WHOLE_SIZE);
    vk::DependencyInfo dependencyInfo =
        vk::DependencyInfo{}
            .setBufferMemoryBarrierCount(1)
            .setPBufferMemoryBarriers(&bufferBarrier);
    commandBuffer.pipelineBarrier2(dependencyInfo);

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedVertexComputeVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    for (unsigned int lodIndex = 0; lodIndex < MAX_LOD_COUNT; lodIndex++)
    {
        auto &buffer =
            storageIndirectBuffers_.front()[renderer_->getCurrentFrameIndex()];
        renderer_->getCommandBuffer().drawIndexedIndirect(
            *buffer.getBuffer(),
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

bool InstancedVertexComputeVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer_->getCurrentFrameIndex()].copyToStaging(
        debugDataBufferSize, vk::PipelineStageFlagBits::eVertexShader |
                                 vk::PipelineStageFlagBits::eFragmentShader
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
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer_->getCurrentFrameIndex()].readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nVertexShaderInvocations =
        debugDataBufferData_[0].nVertexShaderInvocations;
    pipelineStatistics_.nComputeShaderInvocations =
        debugDataBufferData_[0].nComputeShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    pipelineStatistics_.nTriangles =
        (geometry_->meshletLODCounts[*updateData_->lodIndex] / 3) *
        updateData_->instances->size();
    pipelineStatistics_.nDrawCalls = nDrawCalls_;
    return true;
}
