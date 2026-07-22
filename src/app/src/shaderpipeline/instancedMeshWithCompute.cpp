#include "shaderpipeline/instancedMeshWithCompute.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedMeshComputeVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ = dynamic_cast<InstancedMeshComputeShaderPipelineUpdateData *>(
        updateData
    );

    vk::PhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties;
    vk::PhysicalDeviceProperties2 physicalDeviceProperties;
    physicalDeviceProperties.pNext = &meshShaderProperties;
    renderer_->getPhysicalDevice().getProperties2(&physicalDeviceProperties);
    maxTaskShaderThreadCount_ = meshShaderProperties.maxTaskWorkGroupTotalCount;

    return initPipeline(geometry);
}

bool InstancedMeshComputeVulkanShaderPipeline::initPipeline(Geometry *geometry)
{
    geometry_ = geometry;
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

bool InstancedMeshComputeVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedMeshComputeVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    pipelineLayout_ = nullptr;
    computePasses_.clear();
    shutdownBuffers();
    graphicsPipeline_ = nullptr;
    spdlog::debug("Shutdown complete for default vulkan pipeline");
    return true;
}

// TODO - Break up this function
bool InstancedMeshComputeVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedMeshTaskShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshComputeTask")
    );
    VulkanShader *instancedMeshMeshShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshComputeMesh")
    );
    VulkanShader *instancedMeshFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshComputeFragment")
    );

    vk::PipelineShaderStageCreateInfo taskShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eTaskEXT)
            .setModule(instancedMeshTaskShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo meshShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eMeshEXT)
            .setModule(instancedMeshMeshShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(instancedMeshFragmentShader->getShaderModule())
            .setPName("main");

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
        taskShaderStageInfo, meshShaderStageInfo, fragShaderStageInfo
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

    // TODO - move this in with the other buffers (is buffer like)
    std::vector<vk::PushConstantRange> pushConstantRanges = {
        vk::PushConstantRange{}
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eFragment
            )
            .setOffset(0)
            .setSize(sizeof(InstancedMeshCompute::Constants)),
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

bool InstancedMeshComputeVulkanShaderPipeline::initComputeShaderPipeline()
{
    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();

    VulkanShader *lodStatsComputeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMCLODStats")
    );
    VulkanShader *drawCommandComputeShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMCDrawCommand")
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

void InstancedMeshComputeVulkanShaderPipeline::initBufferDescriptions()
{
    // TODO - the dream is having this as pure config
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedMeshCompute::BufferSlots::SCENE)
            .setBufferSize(sizeof(InstancedMeshCompute::SceneBuffer))
            .setData(static_cast<InstancedMeshCompute::SceneBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eCompute
            )
    );
    resourceDescriptions_.push_back(
        UniformBuffer
            .setBufferSlot(InstancedMeshCompute::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedMeshCompute::AnimationBuffer))
            .setData(
                static_cast<InstancedMeshCompute::AnimationBuffer *>(nullptr)
            )
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eCompute
            )
    );
    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(InstancedMeshCompute::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedMeshCompute::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        StorageRead
            .setBufferSlot(InstancedMeshCompute::BufferSlots::MODEL_VERTICES)
            .setBufferSize(sizeof(VertexInputType) * geometry_->vertices.size())
            .setData(geometry_->vertices.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(InstancedMeshCompute::BufferSlots::MESHLETS)
            .setBufferSize(sizeof(meshopt_Meshlet) * geometry_->meshlets.size())
            .setData(geometry_->meshlets.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead
            .setBufferSlot(InstancedMeshCompute::BufferSlots::VERTEX_INDICES)
            .setBufferSize(sizeof(uint32_t) * geometry_->meshletVertices.size())
            .setData(geometry_->meshletVertices.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead
            .setBufferSlot(InstancedMeshCompute::BufferSlots::TRIANGLE_INDICES)
            .setBufferSize(
                sizeof(uint32_t) * geometry_->meshletTrianglesU32.size()
            )
            .setData(geometry_->meshletTrianglesU32.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(InstancedMeshCompute::BufferSlots::INSTANCES)
            .setBufferSize(sizeof(Instance) * MAX_N_INSTANCES)
            .setData(static_cast<Instance *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eCompute
            )
    );
    resourceDescriptions_.push_back(
        StorageRead
            .setBufferSlot(
                InstancedMeshCompute::BufferSlots::MESHLET_BOUNDING_SPHERES
            )
            .setBufferSize(
                sizeof(glm::vec4) * geometry_->meshletBoundingSpheres.size()
            )
            .setData(geometry_->meshletBoundingSpheres.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT
            )
    );
    resourceDescriptions_.push_back(
        ImageDescription()
            .setBufferSlot(InstancedMeshCompute::BufferSlots::TEXTURE)
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
    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS)
            .setBufferSize(sizeof(InstancedMeshCompute::IndirectCommandBuffer))
            .setData(nullptr)
            .setStageFlags(
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eTaskEXT
            )
    );
    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(
                InstancedMeshCompute::BufferSlots::INDIRECT_DRAW_COMMANDS
            )
            .setBufferSize(
                sizeof(InstancedMeshCompute::MeshTasksIndirectCommand) *
                MAX_INDIRECT_COMMAND_COUNT
            )
            .setDeviceUsage(
                StorageReadWrite.deviceUsage |
                vk::BufferUsageFlagBits::eIndirectBuffer
            )
            .setData(nullptr)
            .setStageFlags(
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eTaskEXT
            )
    );
    // TODO - this is an excessive reservation
    resourceDescriptions_.push_back(
        StorageReadWrite
            .setBufferSlot(InstancedMeshCompute::BufferSlots::LOD_INSTANCES)
            .setBufferSize(
                sizeof(ComputedInstance) * MAX_N_INSTANCES * MAX_LOD_COUNT
            )
            .setData(nullptr)
            .setStageFlags(
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT
            )
    );
}

void InstancedMeshComputeVulkanShaderPipeline::initBuffers()
{
    initBufferDescriptions();

    for (auto bufferDescription : resourceDescriptions_)
        addResource(bufferDescription);
}

bool InstancedMeshComputeVulkanShaderPipeline::bind()
{
    spdlog::debug("Binding shader pipeline");

    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::Extent2D &swapChainExtent = renderer_->getSwapChainExtent();

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, *getGraphicsPipeline()
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

void InstancedMeshComputeVulkanShaderPipeline::updateSceneData()
{
    InstancedMeshCompute::SceneBuffer sceneData{};
    sceneData.eyePosition = {
        updateData_->camera->getComponent<TransformComponent>()->getPosition(),
        1.0f
    };
    sceneData.view = *updateData_->camera->getViewMatrixRef();
    sceneData.projection = *updateData_->camera->getProjectionMatrixRef();
    sceneData.instanceCount = updateData_->instances->size();
    sceneData.meshletCount = geometry_->meshletLODCounts[0];
    sceneData.maxGroupsPerDraw = maxTaskShaderThreadCount_;
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
    sceneData.visibilityFunc =
        static_cast<uint32_t>(*updateData_->visibilityFunc);

    resources_[InstancedMeshCompute::BufferSlots::SCENE].update(
        sizeof(sceneData), &sceneData
    );
}

void InstancedMeshComputeVulkanShaderPipeline::updateAnimationData()
{
    InstancedMeshCompute::AnimationBuffer animation{};
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
    resources_[InstancedMeshCompute::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );
}

void InstancedMeshComputeVulkanShaderPipeline::updateInstancesData()
{
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    resources_[InstancedMeshCompute::BufferSlots::INSTANCES].update(
        instancesBufferSize, updateData_->instances->data()
    );
}

void InstancedMeshComputeVulkanShaderPipeline::updateDebugData()
{
    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMeshCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedMeshCompute::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    resources_[InstancedMeshCompute::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );
}

void InstancedMeshComputeVulkanShaderPipeline::resetIndirectDrawCommands()
{
    InstancedMeshCompute::IndirectCommandBuffer resetCommandData{};
    resources_[InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS].update(
        sizeof(InstancedMeshCompute::IndirectCommandBuffer), &resetCommandData
    );

    // Zero out all indirect draw commands so any uninitialised entries are
    // no-ops
    std::vector<InstancedMeshCompute::MeshTasksIndirectCommand>
        resetDrawCommands(
            MAX_INDIRECT_COMMAND_COUNT,
            InstancedMeshCompute::MeshTasksIndirectCommand{}
        );
    resources_[InstancedMeshCompute::BufferSlots::INDIRECT_DRAW_COMMANDS]
        .update(
            sizeof(InstancedMeshCompute::MeshTasksIndirectCommand) *
                MAX_INDIRECT_COMMAND_COUNT,
            resetDrawCommands.data()
        );
}

void InstancedMeshComputeVulkanShaderPipeline::bindComputePipeline(uint pass)
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

void InstancedMeshComputeVulkanShaderPipeline::lodStatsComputePass(
    uint dispatchCount
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

    commandBuffer.dispatch(dispatchCount, 1, 1);

    MemoryBarrierInfo memoryBarrierInfo = {
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eShaderRead |
                             vk::AccessFlagBits2::eShaderWrite,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eComputeShader,
    };
    renderer_->createPipelineMemoryBarrier(memoryBarrierInfo);
}

void InstancedMeshComputeVulkanShaderPipeline::drawCommandComputePass(
    uint dispatchCount
)
{
    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();

    commandBuffer.dispatch(dispatchCount, 1, 1);

    uint32_t frameIndex = renderer_->getCurrentFrameIndex();
    VulkanBuffer *lodInstancesBuffer =
        resources_[InstancedMeshCompute::BufferSlots::LOD_INSTANCES].asBuffer();
    VulkanBuffer *indirectBuffer =
        resources_[InstancedMeshCompute::BufferSlots::INDIRECT_COMMANDS]
            .asBuffer();

    BufferBarrierInfo barrierInfo;

    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eIndirectCommandRead |
                             vk::AccessFlagBits2::eShaderRead,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eDrawIndirect |
                            vk::PipelineStageFlagBits2::eTaskShaderEXT,
        .buffer = lodInstancesBuffer->getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);

    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eShaderRead,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eTaskShaderEXT,
        .buffer = indirectBuffer->getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);

    // Barrier for the indirect draw commands buffer
    VulkanBuffer *indirectDrawBuffer =
        resources_[InstancedMeshCompute::BufferSlots::INDIRECT_DRAW_COMMANDS]
            .asBuffer();
    barrierInfo = BufferBarrierInfo{
        .sourceAccess = vk::AccessFlagBits2::eShaderWrite,
        .destinationAccess = vk::AccessFlagBits2::eIndirectCommandRead,
        .sourceStage = vk::PipelineStageFlagBits2::eComputeShader,
        .destinationStage = vk::PipelineStageFlagBits2::eDrawIndirect,
        .buffer = indirectDrawBuffer->getBuffer(frameIndex)
    };
    addBarrier(barrierInfo);
    setBarrier();
}

bool InstancedMeshComputeVulkanShaderPipeline::update()
{

    std::chrono::time_point now = std::chrono::steady_clock::now();

    uint32_t instanceCount =
        static_cast<uint32_t>(updateData_->instances->size());
    uint32_t lodStatsComputePassGroupCount =
        (instanceCount + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;

    updateSceneData();
    updateAnimationData();
    updateInstancesData();
    updateDebugData();

    resetIndirectDrawCommands();

    bindComputePipeline(0);
    lodStatsComputePass(lodStatsComputePassGroupCount);
    bindComputePipeline(1);
    drawCommandComputePass(1);

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedMeshComputeVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    uint32_t frameIndex = renderer_->getCurrentFrameIndex();

    // Calculate maximum possible work across all LODs to bound the loop.
    // The reset ensures any uninitialised indirect commands are zero (no-ops).
    uint32_t maximumPossibleWork = 0;
    for (uint32_t lodIndex = 0;
         lodIndex < static_cast<uint32_t>(geometry_->meshletLODCounts.size());
         lodIndex++)
    {
        maximumPossibleWork +=
            static_cast<uint32_t>(geometry_->meshletLODCounts[lodIndex]);
    }
    maximumPossibleWork *=
        static_cast<uint32_t>(updateData_->instances->size());
    uint32_t maximumPossibleGroups =
        (maximumPossibleWork + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;
    uint32_t maximumDrawCalls =
        (maximumPossibleGroups + maxTaskShaderThreadCount_ - 1) /
        maxTaskShaderThreadCount_;

    // Cap to avoid over-iteration; the compute shader won't write more than
    // MAX_INDIRECT_COMMAND_COUNT entries anyway.
    maximumDrawCalls = std::min(
        maximumDrawCalls, static_cast<uint32_t>(MAX_INDIRECT_COMMAND_COUNT)
    );

    vk::raii::CommandBuffer &commandBuffer = renderer_->getCommandBuffer();
    vk::raii::Buffer &indirectDrawBuffer =
        *resources_[InstancedMeshCompute::BufferSlots::INDIRECT_DRAW_COMMANDS]
             .asBuffer()
             ->getBuffer(frameIndex);

    uint32_t workOffset = 0;
    nDrawCalls_ = 0;
    for (uint32_t drawIndex = 0; drawIndex < maximumDrawCalls; drawIndex++)
    {
        // Each indirect command is MeshTasksIndirectCommand (3 x uint32 = 12
        // bytes), matching vk::DrawMeshTasksIndirectCommandEXT layout.
        vk::DeviceSize commandOffset =
            drawIndex * sizeof(InstancedMeshCompute::MeshTasksIndirectCommand);

        // Calculate work range for this draw based on groups this command will
        // dispatch. The actual group count comes from the GPU-written indirect
        // command; we use maxTaskShaderThreadCount_ as the per-draw cap for
        // push constant tracking.
        uint32_t currentWork = maxTaskShaderThreadCount_ * TS_GROUP_SIZE;

        constants = {};
        constants.workStart = workOffset;
        constants.workCount = currentWork;
        constants.fragmentColourType = *updateData_->fragmentColourType;
        commandBuffer.pushConstants(
            pipelineLayout_,
            vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eFragment,
            0, sizeof(InstancedMeshCompute::Constants), &constants
        );

        commandBuffer.drawMeshTasksIndirectEXT(
            indirectDrawBuffer, commandOffset, 1,
            sizeof(vk::DrawMeshTasksIndirectCommandEXT)
        );

        workOffset += currentWork;
        nDrawCalls_++;
    }

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedMeshComputeVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMeshCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    resources_[InstancedMeshCompute::BufferSlots::DEBUG]
        .asBuffer()
        ->copyToStaging(
            debugDataBufferSize,
            vk::PipelineStageFlagBits::eComputeShader |
                vk::PipelineStageFlagBits::eTaskShaderEXT |
                vk::PipelineStageFlagBits::eMeshShaderEXT |
                vk::PipelineStageFlagBits::eFragmentShader,
            renderer_->getCurrentFrameIndex()
        );
    return true;
}

PipelineStatistics
InstancedMeshComputeVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedMeshComputeVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return true;

    pipelineStatistics_.pipelineType = PipelineStatistics::PipelineType::MESH;

    // TODO - debug buffer should be it's own allocation tbh, or I track it as
    // a pointer
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMeshCompute::DebugDataBuffer) *
        debugDataBufferData_.size();
    resources_[InstancedMeshCompute::BufferSlots::DEBUG].asBuffer()->readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nTaskShaderInvocations =
        debugDataBufferData_[0].nTaskShaderInvocations;
    pipelineStatistics_.nMeshShaderInvocations =
        debugDataBufferData_[0].nMeshShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    pipelineStatistics_.nTriangles = debugDataBufferData_[0].nTriangles;
    pipelineStatistics_.nDrawCalls = nDrawCalls_;
    return true;
}
