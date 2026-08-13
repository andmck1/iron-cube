#include "shaderpipeline/meshDXExample.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool MeshDXExampleVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<MeshDXExampleShaderPipelineUpdateData *>(updateData);

    vk::PhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties;
    vk::PhysicalDeviceProperties2 physicalDeviceProperties;
    physicalDeviceProperties.pNext = &meshShaderProperties;
    renderer_->getPhysicalDevice().getProperties2(&physicalDeviceProperties);
    maxTaskShaderThreadCount_ = meshShaderProperties.maxTaskWorkGroupTotalCount;
    geometry_ = geometry;

    return true;
}

bool MeshDXExampleVulkanShaderPipeline::initPipeline()
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

bool MeshDXExampleVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    shutdown();
    return initPipeline();
}

bool MeshDXExampleVulkanShaderPipeline::shutdown()
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

// TODO - Break up this function
bool MeshDXExampleVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *taskShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("meshDXExampleTask")
    );
    VulkanShader *meshShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("meshDXExampleMesh")
    );
    VulkanShader *fragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("meshDXExampleFragment")
    );

    vk::PipelineShaderStageCreateInfo taskShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eTaskEXT)
            .setModule(taskShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo meshShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eMeshEXT)
            .setModule(meshShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(fragmentShader->getShaderModule())
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
            .setSize(sizeof(MeshDXExample::Constants)),
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

void MeshDXExampleVulkanShaderPipeline::initBufferDescriptions()
{
    // TODO - the dream is having this as pure config
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(MeshDXExample::BufferSlots::SCENE)
            .setBufferSize(sizeof(MeshDXExample::SceneBuffer))
            .setData(static_cast<MeshDXExample::SceneBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(MeshDXExample::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(MeshDXExample::AnimationBuffer))
            .setData(static_cast<MeshDXExample::AnimationBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT
            )
    );
    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(MeshDXExample::BufferSlots::DEBUG)
            .setBufferSize(sizeof(MeshDXExample::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(MeshDXExample::BufferSlots::MODEL_VERTICES)
            .setBufferSize(sizeof(VertexInputType) * geometry_->vertices.size())
            .setData(geometry_->vertices.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(MeshDXExample::BufferSlots::MESHLETS)
            .setBufferSize(sizeof(meshopt_Meshlet) * geometry_->meshlets.size())
            .setData(geometry_->meshlets.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT
            )
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(MeshDXExample::BufferSlots::VERTEX_INDICES)
            .setBufferSize(sizeof(uint32_t) * geometry_->meshletVertices.size())
            .setData(geometry_->meshletVertices.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(MeshDXExample::BufferSlots::TRIANGLE_INDICES)
            .setBufferSize(
                sizeof(uint32_t) * geometry_->meshletTrianglesU32.size()
            )
            .setData(geometry_->meshletTrianglesU32.data())
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT)
    );
    resourceDescriptions_.push_back(
        StorageRead.setBufferSlot(MeshDXExample::BufferSlots::INSTANCES)
            .setBufferSize(sizeof(Instance) * MAX_N_INSTANCES)
            .setData(static_cast<Instance *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT
            )
    );
    resourceDescriptions_.push_back(
        StorageRead
            .setBufferSlot(MeshDXExample::BufferSlots::MESHLET_BOUNDING_SPHERES)
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
            .setBufferSlot(MeshDXExample::BufferSlots::TEXTURE)
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

void MeshDXExampleVulkanShaderPipeline::initBuffers()
{
    initBufferDescriptions();

    for (auto bufferDescription : resourceDescriptions_)
        addResource(bufferDescription);
}

bool MeshDXExampleVulkanShaderPipeline::bind()
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

bool MeshDXExampleVulkanShaderPipeline::update()
{

    std::chrono::time_point now = std::chrono::steady_clock::now();
    MeshDXExample::SceneBuffer sceneData{};
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
    sceneData.ambientLight = *updateData_->ambientLight;
    sceneData.directionalLight = *updateData_->directionalLight;

    resources_[MeshDXExample::BufferSlots::SCENE].update(
        sizeof(sceneData), &sceneData
    );

    MeshDXExample::AnimationBuffer animation{};
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
    resources_[MeshDXExample::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );

    // This takes up most of the frame time
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    resources_[MeshDXExample::BufferSlots::INSTANCES].update(
        instancesBufferSize, updateData_->instances->data()
    );

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(MeshDXExample::DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {MeshDXExample::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    resources_[MeshDXExample::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool MeshDXExampleVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();

    uint32_t instanceCount =
        static_cast<uint32_t>(updateData_->instances->size());

    uint32_t instancesPerDispatch = maxTaskShaderThreadCount_ * TS_GROUP_SIZE;
    uint32_t dispatchCount =
        (instanceCount + instancesPerDispatch - 1) / instancesPerDispatch;

    nDrawCalls_ = dispatchCount;

    for (uint32_t i = 0; i < dispatchCount; ++i)
    {
        uint32_t offset = instancesPerDispatch * i;
        uint32_t count = std::min(instanceCount - offset, instancesPerDispatch);

        constants = {};
        constants.workStart = offset;
        constants.workCount = count;
        constants.fragmentColourType = *updateData_->fragmentColourType;

        renderer_->getCommandBuffer().pushConstants(
            pipelineLayout_,
            vk::ShaderStageFlagBits::eTaskEXT |
                vk::ShaderStageFlagBits::eMeshEXT |
                vk::ShaderStageFlagBits::eFragment,
            0, sizeof(MeshDXExample::Constants), &constants
        );

        uint32_t groupCount = (count + TS_GROUP_SIZE - 1) / TS_GROUP_SIZE;
        renderer_->getCommandBuffer().drawMeshTasksEXT(groupCount, 1, 1);
    }

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();
    return true;
}

bool MeshDXExampleVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(MeshDXExample::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[MeshDXExample::BufferSlots::DEBUG].asBuffer()->copyToStaging(
        debugDataBufferSize,
        vk::PipelineStageFlagBits::eTaskShaderEXT |
            vk::PipelineStageFlagBits::eMeshShaderEXT |
            vk::PipelineStageFlagBits::eFragmentShader,
        renderer_->getCurrentFrameIndex()
    );
    return true;
}

PipelineStatistics MeshDXExampleVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool MeshDXExampleVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return true;

    vk::DeviceSize debugDataBufferSize =
        sizeof(MeshDXExample::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[MeshDXExample::BufferSlots::DEBUG].asBuffer()->readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    // TODO - possible to track meshlets? would requiring copying the data back
    uint32_t nInstances = updateData_->instances->size();
    pipelineStatistics_.stats = {
        {"N Instances", nInstances},
        {"N Max Vertices", nInstances * geometry_->vertexCounts[0]},
        {"N Max Meshlets", geometry_->meshletLODCounts[0] * nInstances},
        {"N TS Invocations", debugDataBufferData_[0].nTaskShaderInvocations},
        {"N MS Invocations", debugDataBufferData_[0].nMeshShaderInvocations},
        {"N FS Invocations",
         debugDataBufferData_[0].nFragmentShaderInvocations},
        {"N Triangles", debugDataBufferData_[0].nTriangles},
        {"N Draw Calls", nDrawCalls_},
    };

    return true;
}
