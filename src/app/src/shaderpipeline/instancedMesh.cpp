#include "shaderpipeline/instancedMesh.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedMeshVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<InstancedMeshShaderPipelineUpdateData *>(updateData);

    vk::PhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties;
    vk::PhysicalDeviceProperties2 physicalDeviceProperties;
    physicalDeviceProperties.pNext = &meshShaderProperties;
    renderer_->getPhysicalDevice().getProperties2(&physicalDeviceProperties);
    maxTaskShaderThreadCount_ = meshShaderProperties.maxTaskWorkGroupTotalCount;

    return initPipeline(geometry);
}

bool InstancedMeshVulkanShaderPipeline::initPipeline(Geometry *geometry)
{
    geometry_ = geometry;
    initUniformBuffers();
    initStorageBuffers();
    if (!initDescriptorLayout())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
        return false;
    return true;
}

bool InstancedMeshVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedMeshVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for default vulkan pipeline");
    return true;
}

// TODO - Break up this function
bool InstancedMeshVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedMeshTaskShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshTask")
    );
    VulkanShader *instancedMeshMeshShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshMesh")
    );
    VulkanShader *instancedMeshFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedMeshFragment")
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
    vk::PushConstantRange pushConstantRange =
        vk::PushConstantRange{}
            .setStageFlags(vk::ShaderStageFlagBits::eTaskEXT)
            .setOffset(0)
            .setSize(sizeof(InstancedMesh::TaskWorkBuffer));

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

void InstancedMeshVulkanShaderPipeline::initUniformBuffers()
{
    BufferDescription sceneDesc = UniformBuffer;
    sceneDesc.stageFlags =
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(
        sceneDesc, sizeof(InstancedMesh::SceneBuffer),
        static_cast<InstancedMesh::SceneBuffer *>(nullptr)
    );

    BufferDescription animDesc = UniformBuffer;
    animDesc.stageFlags = vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(
        animDesc, sizeof(InstancedMesh::AnimationBuffer),
        static_cast<InstancedMesh::AnimationBuffer *>(nullptr)
    );
}

void InstancedMeshVulkanShaderPipeline::initStorageBuffers()
{
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMesh::DebugDataBuffer) * debugDataBufferData_.size();
    BufferDescription debugDesc = StorageReadWrite;
    debugDesc.stageFlags = vk::ShaderStageFlagBits::eTaskEXT |
                           vk::ShaderStageFlagBits::eMeshEXT |
                           vk::ShaderStageFlagBits::eFragment;
    addBuffer(debugDesc, debugDataBufferSize, debugDataBufferData_.data());

    vk::DeviceSize vertexBufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    BufferDescription verticesDesc = StorageRead;
    verticesDesc.stageFlags = vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(verticesDesc, vertexBufferSize, geometry_->vertices.data());

    vk::DeviceSize meshletBufferSize =
        sizeof(meshopt_Meshlet) * geometry_->meshlets.size();
    BufferDescription meshletsDesc = StorageRead;
    meshletsDesc.stageFlags = vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(meshletsDesc, meshletBufferSize, geometry_->meshlets.data());

    vk::DeviceSize meshletVerticesBufferSize =
        sizeof(uint32_t) * geometry_->meshletVertices.size();
    BufferDescription meshletVertsDesc = StorageRead;
    meshletVertsDesc.stageFlags = vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(
        meshletVertsDesc, meshletVerticesBufferSize,
        geometry_->meshletVertices.data()
    );

    vk::DeviceSize meshletTrianglesBufferSize =
        sizeof(uint32_t) * geometry_->meshletTrianglesU32.size();
    BufferDescription meshletTrisDesc = StorageRead;
    meshletTrisDesc.stageFlags = vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(
        meshletTrisDesc, meshletTrianglesBufferSize,
        geometry_->meshletTrianglesU32.data()
    );

    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
        MAX_N_INSTANCE_ROWS;
    BufferDescription instancesDesc = StorageRead;
    instancesDesc.stageFlags =
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(instancesDesc, instancesBufferSize, nullptr);

    vk::DeviceSize meshletBoundingSpheresBufferSize =
        sizeof(glm::vec4) * geometry_->meshletBoundingSpheres.size();
    BufferDescription boundingSpheresDesc = StorageRead;
    boundingSpheresDesc.stageFlags =
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT;
    addBuffer(
        boundingSpheresDesc, meshletBoundingSpheresBufferSize,
        geometry_->meshletBoundingSpheres.data()
    );
}

bool InstancedMeshVulkanShaderPipeline::bind()
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

bool InstancedMeshVulkanShaderPipeline::update()
{

    std::chrono::time_point now = std::chrono::steady_clock::now();
    InstancedMesh::SceneBuffer sceneData{};
    sceneData.eyePosition = {
        updateData_->camera->getComponent<TransformComponent>()->getPosition(),
        1.0f
    };
    sceneData.view = *updateData_->camera->getViewMatrixRef();
    sceneData.projection = *updateData_->camera->getProjectionMatrixRef();
    sceneData.instanceCount = updateData_->instances->size();
    sceneData.meshletCount = geometry_->meshletLODCounts[0];
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

    buffers_[InstancedMesh::BufferSlots::SCENE].update(
        sizeof(sceneData), &sceneData
    );

    InstancedMesh::AnimationBuffer animation{};
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
    buffers_[InstancedMesh::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );

    // This takes up most of the frame time
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    buffers_[InstancedMesh::BufferSlots::INSTANCES].update(
        instancesBufferSize, updateData_->instances->data()
    );

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMesh::DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedMesh::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    buffers_[InstancedMesh::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedMeshVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();

    uint32_t meshletCount =
        static_cast<uint32_t>(geometry_->meshletLODCounts[0]);
    uint32_t instanceCount =
        static_cast<uint32_t>(updateData_->instances->size());
    uint32_t maximumPossibleWork = meshletCount * instanceCount;
    uint32_t maximumPossibleThreads =
        (maximumPossibleWork + AS_GROUP_SIZE - 1) / AS_GROUP_SIZE;

    uint32_t workRemaining = maximumPossibleWork;
    uint32_t threadsRemaining = maximumPossibleThreads;
    uint32_t workOffset = 0;

    nDrawCalls_ = 0;
    while (threadsRemaining > 0)
    {
        nDrawCalls_++;
        uint32_t currentThreads =
            std::min(threadsRemaining, maxTaskShaderThreadCount_);
        uint32_t currentWork = currentThreads * AS_GROUP_SIZE;

        InstancedMesh::TaskWorkBuffer workData{};
        workData.workStart = workOffset;
        workData.workCount = currentWork;
        renderer_->getCommandBuffer().pushConstants(
            pipelineLayout_, vk::ShaderStageFlagBits::eTaskEXT, 0,
            sizeof(InstancedMesh::TaskWorkBuffer), &workData
        );

        renderer_->getCommandBuffer().drawMeshTasksEXT(currentThreads, 1, 1);

        workOffset += currentWork;
        workRemaining -= currentWork;
        threadsRemaining -= currentThreads;
        // TODO - wonder if you want to ensure that this is a nice number? eg
        // 256,
    }
    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool InstancedMeshVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMesh::DebugDataBuffer) * debugDataBufferData_.size();
    buffers_[InstancedMesh::BufferSlots::DEBUG].copyToStaging(
        debugDataBufferSize,
        vk::PipelineStageFlagBits::eTaskShaderEXT |
            vk::PipelineStageFlagBits::eMeshShaderEXT |
            vk::PipelineStageFlagBits::eFragmentShader,
        renderer_->getCurrentFrameIndex()
    );
    return true;
}

PipelineStatistics InstancedMeshVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedMeshVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return true;

    pipelineStatistics_.pipelineType = PipelineStatistics::PipelineType::MESH;

    // TODO - debug buffer should be it's own allocation tbh, or I track it as
    // a pointer
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedMesh::DebugDataBuffer) * debugDataBufferData_.size();
    buffers_[InstancedMesh::BufferSlots::DEBUG].readData(
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
