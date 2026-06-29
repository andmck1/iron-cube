#include "shaderpipeline/instancedMesh.h"
#include "core_buffertypes/buffertypes.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_float4x4.hpp"

bool InstancedMeshVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    updateData_ = updateData;
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
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

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

void InstancedMeshVulkanShaderPipeline::initUniformBuffers()
{
    addUniformBuffer(
        sizeof(SceneBuffer), static_cast<SceneBuffer *>(nullptr),
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );
}

void InstancedMeshVulkanShaderPipeline::initStorageBuffers()
{
    const InstancedMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const InstancedMeshShaderPipelineUpdateData &>(
            *updateData_
        );

    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    addStorageRWBuffer(
        debugDataBufferSize, debugDataBufferData_.data(),
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT |
            vk::ShaderStageFlagBits::eFragment
    );

    vk::DeviceSize vertexBufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    addStorageRBuffer(
        vertexBufferSize, geometry_->vertices.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletBufferSize =
        sizeof(meshopt_Meshlet) * geometry_->meshlets.size();
    addStorageRBuffer(
        meshletBufferSize, geometry_->meshlets.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletVerticesBufferSize =
        sizeof(uint32_t) * geometry_->meshletVertices.size();
    addStorageRBuffer(
        meshletVerticesBufferSize, geometry_->meshletVertices.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletTrianglesBufferSize =
        sizeof(uint32_t) * geometry_->meshletTrianglesU32.size();
    addStorageRBuffer(
        meshletTrianglesBufferSize, geometry_->meshletTrianglesU32.data(),
        vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize instancesBufferSize =
        sizeof(glm::mat4x4) * defaultUpdateData.instances->size();
    addStorageRBuffer(
        instancesBufferSize, defaultUpdateData.instances->data(),
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );

    vk::DeviceSize meshletBoundingSpheresBufferSize =
        sizeof(glm::vec4) * geometry_->meshletBoundingSpheres.size();
    addStorageRBuffer(
        meshletBoundingSpheresBufferSize,
        geometry_->meshletBoundingSpheres.data(),
        vk::ShaderStageFlagBits::eTaskEXT
    );
}

bool InstancedMeshVulkanShaderPipeline::bind()
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

bool InstancedMeshVulkanShaderPipeline::update()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    vk::raii::CommandBuffer &commandBuffer = renderer->getCommandBuffer();
    commandBuffer.resetQueryPool(queryPool_, 0, 1);

    const InstancedMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const InstancedMeshShaderPipelineUpdateData &>(
            *updateData_
        );

    // TODO - hardcoded indices!!
    // scene data (uniform buffer index 0)
    SceneBuffer sceneData{};
    sceneData.view = *defaultUpdateData.camera->getViewMatrixRef();
    sceneData.projection = *defaultUpdateData.camera->getProjectionMatrixRef();
    sceneData.instanceCount = defaultUpdateData.instances->size();
    sceneData.meshletCount = geometry_->meshlets.size();

    std::array<FrustumPlane, 6> frustumPlanes =
        defaultUpdateData.camera->getFrustumPlanes();
    FrustumCone frustumCone = defaultUpdateData.camera->getFrustumCone();

    for (int planeIndex = 0; planeIndex < 6; planeIndex++)
    {
        sceneData.frustum.planes[planeIndex] = frustumPlanes[planeIndex];
    }

    sceneData.frustum.sphere = defaultUpdateData.camera->getFrustumSphere();
    sceneData.frustum.cone.tip = frustumCone.tip;
    sceneData.frustum.cone.height = frustumCone.height;
    sceneData.frustum.cone.direction = frustumCone.direction;
    sceneData.frustum.cone.angle = frustumCone.angle;
    sceneData.visibilityFunc = *defaultUpdateData.visibilityFunc;

    memcpy(
        uniformBuffers_[0][renderer->getCurrentFrameIndex()].getMappedPointer(),
        &sceneData, sizeof(sceneData)
    );

    // instances data (storage buffer index 4)
    vk::DeviceSize instancesBufferSize =
        sizeof(Instance) * defaultUpdateData.instances->size();
    storageRBuffers_[5][renderer->getCurrentFrameIndex()].update(
        instancesBufferSize, defaultUpdateData.instances->data()
    );

    // debug data (storage buffer index 5, last)
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {DebugDataBuffer()};
    storageRBuffers_.front()[renderer->getCurrentFrameIndex()].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    return true;
}

bool InstancedMeshVulkanShaderPipeline::draw()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    // TODO - wonder if you want to ensure that this is a nice number? eg 256,
    // etc
    // TODO - can't be calling this every frame
    const InstancedMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const InstancedMeshShaderPipelineUpdateData &>(
            *updateData_
        );
    uint32_t meshletCount = static_cast<uint32_t>(geometry_->meshlets.size());
    uint32_t instanceCount =
        static_cast<uint32_t>(defaultUpdateData.instances->size());
    uint32_t threadGroupCountX =
        ((meshletCount * instanceCount) / AS_GROUP_SIZE) + 1;

    renderer->getCommandBuffer().drawMeshTasksEXT(threadGroupCountX, 1, 1);

    return true;
}

bool InstancedMeshVulkanShaderPipeline::postRender()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer->getCurrentFrameIndex()].copyToStaging(
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
    isDebugQuery_ = false;

    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    // TODO - this is a bit extreme
    renderer->waitAllFramesComplete();

    pipelineStatistics_.pipelineType = PipelineStatistics::PipelineType::MESH;

    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageRWBuffers_.front()[renderer->getCurrentFrameIndex()].readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );
    pipelineStatistics_.nTaskShaderInvocations =
        debugDataBufferData_[0].nTaskShaderInvocations;
    pipelineStatistics_.nMeshShaderInvocations =
        debugDataBufferData_[0].nMeshShaderInvocations;
    pipelineStatistics_.nFragmentShaderInvocations =
        debugDataBufferData_[0].nFragmentShaderInvocations;
    return true;
}
