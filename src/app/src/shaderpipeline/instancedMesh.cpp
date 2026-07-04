#include "shaderpipeline/instancedMesh.h"
#include "core_buffertypes/buffertypes.h"
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
    VulkanShader *defaultTaskShader =
        dynamic_cast<VulkanShader *>(shaderManager->getShader("defaultTask"));
    VulkanShader *defaultMeshShader =
        dynamic_cast<VulkanShader *>(shaderManager->getShader("defaultMesh"));
    VulkanShader *defaultFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("defaultFragment")
    );

    vk::PipelineShaderStageCreateInfo taskShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eTaskEXT)
            .setModule(defaultTaskShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo meshShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eMeshEXT)
            .setModule(defaultMeshShader->getShaderModule())
            .setPName("main");

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo =
        vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eFragment)
            .setModule(defaultFragmentShader->getShaderModule())
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

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo{}
            .setSetLayoutCount(1)
            .setPSetLayouts(&*descriptorSetLayout_)
            .setPushConstantRangeCount(0);

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
    addBuffer<VulkanBufferUniform>(
        uniformBuffers_, sizeof(SceneBuffer),
        static_cast<SceneBuffer *>(nullptr),
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );
    sceneDataUniformBufferIndex_ = 0;
}

void InstancedMeshVulkanShaderPipeline::initStorageBuffers()
{
    // TODO - this indices are still hardcoded....they need to be consistently
    // defined somehow
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    addBuffer<VulkanBufferRWStorage>(
        storageRWBuffers_, debugDataBufferSize, debugDataBufferData_.data(),
        vk::ShaderStageFlagBits::eAll
    );
    debugDataStorageRWBufferIndex_ = 0;

    vk::DeviceSize vertexBufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, vertexBufferSize, geometry_->vertices.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletBufferSize =
        sizeof(meshopt_Meshlet) * geometry_->meshlets.size();
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, meshletBufferSize, geometry_->meshlets.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletVerticesBufferSize =
        sizeof(uint32_t) * geometry_->meshletVertices.size();
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, meshletVerticesBufferSize,
        geometry_->meshletVertices.data(), vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletTrianglesBufferSize =
        sizeof(uint32_t) * geometry_->meshletTrianglesU32.size();
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, meshletTrianglesBufferSize,
        geometry_->meshletTrianglesU32.data(), vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * MAX_N_INSTANCE_COLS * MAX_N_INSTANCE_LAYERS *
        MAX_N_INSTANCE_ROWS;
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, instancesBufferSize, updateData_->instances->data(),
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );
    instancesStorageRBufferIndex_ = 4;

    vk::DeviceSize meshletBoundingSpheresBufferSize =
        sizeof(glm::vec4) * geometry_->meshletBoundingSpheres.size();
    addBuffer<VulkanBufferRStorage>(
        storageRBuffers_, meshletBoundingSpheresBufferSize,
        geometry_->meshletBoundingSpheres.data(),
        vk::ShaderStageFlagBits::eTaskEXT
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
    SceneBuffer sceneData{};
    sceneData.view = *updateData_->camera->getViewMatrixRef();
    sceneData.projection = *updateData_->camera->getProjectionMatrixRef();
    sceneData.instanceCount = updateData_->instances->size();
    sceneData.meshletCount = geometry_->meshlets.size();
    for (unsigned int lodIndex = 0;
         lodIndex < geometry_->meshletLODOffsets.size(); lodIndex++)
    {
        sceneData.meshletLODOffsets[lodIndex] =
            geometry_->meshletLODOffsets[lodIndex];
        sceneData.meshletLODCounts[lodIndex] =
            geometry_->meshletLODCounts[lodIndex];
    }

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

    uniformBuffers_[0][renderer_->getCurrentFrameIndex()].update(
        sizeof(sceneData), &sceneData
    );

    // This takes up most of the frame time
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * updateData_->instances->size();
    storageRBuffers_[instancesStorageRBufferIndex_]
                    [renderer_->getCurrentFrameIndex()]
                        .update(
                            instancesBufferSize, updateData_->instances->data()
                        );

    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    storageRWBuffers_[debugDataStorageRWBufferIndex_]
                     [renderer_->getCurrentFrameIndex()]
                         .update(
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

    // etc
    uint32_t meshletCount = static_cast<uint32_t>(geometry_->meshlets.size());
    uint32_t instanceCount =
        static_cast<uint32_t>(updateData_->instances->size());
    uint32_t threadGroupCountX =
        ((meshletCount * instanceCount) / AS_GROUP_SIZE) + 1;

    renderer_->getCommandBuffer().drawMeshTasksEXT(threadGroupCountX, 1, 1);
    // TODO - wonder if you want to ensure that this is a nice number? eg 256,

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
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer_->getCurrentFrameIndex()].copyToStaging(
        debugDataBufferSize, vk::PipelineStageFlagBits::eTaskShaderEXT |
                                 vk::PipelineStageFlagBits::eMeshShaderEXT |
                                 vk::PipelineStageFlagBits::eFragmentShader
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
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer_->getCurrentFrameIndex()].readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nTaskShaderInvocations =
        debugDataBufferData_[0].nTaskShaderInvocations;
    pipelineStatistics_.nMeshShaderInvocations =
        debugDataBufferData_[0].nMeshShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    pipelineStatistics_.nTriangles = debugDataBufferData_[0].nTriangles;
    return true;
}
