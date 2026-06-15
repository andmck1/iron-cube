#include "defaultMesh.h"
#include "core_buffertypes/buffertypes.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"

bool DefaultVulkanMeshShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    updateData_ = updateData;
    return initPipeline(geometry);
}

// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
bool DefaultVulkanMeshShaderPipeline::initPipeline(Geometry *geometry)
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
    if (!createQueryPool())
        return false;
    return true;
}

bool DefaultVulkanMeshShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool DefaultVulkanMeshShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down default vulkan pipeline");
    uniformBuffers_.clear();
    storageBuffers_.clear();

    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    spdlog::debug("Shutdown complete for default vulkan pipeline");
    return true;
}

// TODO - Break up this function
bool DefaultVulkanMeshShaderPipeline::initGraphicsPipeline()
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

bool DefaultVulkanMeshShaderPipeline::initDescriptorLayout()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    uint32_t bindingIndex = 0;

    for (const auto &stageFlags : uniformBufferStageFlags_)
    {
        bindings.emplace_back(
            vk::DescriptorSetLayoutBinding{}
                .setBinding(bindingIndex++)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setStageFlags(stageFlags)
        );
    }

    for (const auto &stageFlags : storageBufferStageFlags_)
    {
        bindings.emplace_back(
            vk::DescriptorSetLayoutBinding{}
                .setBinding(bindingIndex++)
                .setDescriptorType(vk::DescriptorType::eStorageBuffer)
                .setDescriptorCount(1)
                .setStageFlags(stageFlags)
        );
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo =
        vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount(bindings.size())
            .setPBindings(bindings.data());

    descriptorSetLayout_ =
        vk::raii::DescriptorSetLayout(renderer->getDevice(), layoutInfo);
    return true;
}

void DefaultVulkanMeshShaderPipeline::initUniformBuffers()
{
    addUniformBuffer(
        sizeof(SceneBuffer), static_cast<SceneBuffer *>(nullptr),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );
}

void DefaultVulkanMeshShaderPipeline::initStorageBuffers()
{
    const DefaultMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const DefaultMeshShaderPipelineUpdateData &>(*updateData_);

    vk::DeviceSize vertexBufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    vk::DeviceSize meshletBufferSize =
        sizeof(meshopt_Meshlet) * geometry_->meshlets.size();
    vk::DeviceSize meshletVerticesBufferSize =
        sizeof(uint32_t) * geometry_->meshletVertices.size();
    vk::DeviceSize meshletTrianglesBufferSize =
        sizeof(uint32_t) * geometry_->meshletTrianglesU32.size();
    vk::DeviceSize instancesBufferSize =
        sizeof(glm::mat4x4) * defaultUpdateData.instances->size();
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();

    vk::BufferUsageFlagBits bufferType =
        vk::BufferUsageFlagBits::eStorageBuffer;

    addStorageBuffer(
        vertexBufferSize, geometry_->vertices.data(), bufferType, false,
        vk::ShaderStageFlagBits::eMeshEXT
    );
    addStorageBuffer(
        meshletBufferSize, geometry_->meshlets.data(), bufferType, false,
        vk::ShaderStageFlagBits::eMeshEXT
    );
    addStorageBuffer(
        meshletVerticesBufferSize, geometry_->meshletVertices.data(),
        bufferType, false, vk::ShaderStageFlagBits::eMeshEXT
    );
    addStorageBuffer(
        meshletTrianglesBufferSize, geometry_->meshletTrianglesU32.data(),
        bufferType, false, vk::ShaderStageFlagBits::eMeshEXT
    );
    addStorageBuffer(
        instancesBufferSize, defaultUpdateData.instances->data(), bufferType,
        false, vk::ShaderStageFlagBits::eMeshEXT
    );
    addStorageBuffer(
        debugDataBufferSize, debugDataBufferData_.data(), bufferType, true,
        vk::ShaderStageFlagBits::eTaskEXT | vk::ShaderStageFlagBits::eMeshEXT
    );
}

bool DefaultVulkanMeshShaderPipeline::createDescriptorPool()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    std::vector<vk::DescriptorPoolSize> poolSizes = {
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(
                renderer->MAX_FRAMES_IN_FLIGHT * uniformBuffers_.size()
            ),
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(
                renderer->MAX_FRAMES_IN_FLIGHT * storageBuffers_.size()
            )
    };

    vk::DescriptorPoolCreateInfo poolInfo =
        vk::DescriptorPoolCreateInfo{}
            .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(renderer->MAX_FRAMES_IN_FLIGHT)
            .setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
            .setPPoolSizes(poolSizes.data());

    descriptorPool_ = vk::raii::DescriptorPool(renderer->getDevice(), poolInfo);
    return true;
}

bool DefaultVulkanMeshShaderPipeline::initDescriptorSets()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    if (!createDescriptorPool())
        return false;

    std::vector<vk::DescriptorSetLayout> layouts(
        renderer->MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout_
    );
    vk::DescriptorSetAllocateInfo allocInfo =
        vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(descriptorPool_)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

    descriptorSets_ = renderer->getDevice().allocateDescriptorSets(allocInfo);

    for (size_t frameIndex = 0; frameIndex < renderer->MAX_FRAMES_IN_FLIGHT;
         frameIndex++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        std::vector<vk::DescriptorBufferInfo> bufferInfos;

        size_t totalBuffers = uniformBuffers_.size() + storageBuffers_.size();
        descriptorWrites.reserve(totalBuffers);
        bufferInfos.reserve(totalBuffers);

        uint32_t bindingIndex = 0;

        for (size_t bufferIndex = 0; bufferIndex < uniformBuffers_.size();
             bufferIndex++)
        {
            addBufferWrite(
                descriptorSets_[frameIndex], bindingIndex++,
                vk::DescriptorType::eUniformBuffer,
                uniformBuffers_[bufferIndex][frameIndex].getBuffer(),
                VK_WHOLE_SIZE, descriptorWrites, bufferInfos
            );
        }

        for (size_t bufferIndex = 0; bufferIndex < storageBuffers_.size();
             bufferIndex++)
        {
            addBufferWrite(
                descriptorSets_[frameIndex], bindingIndex++,
                vk::DescriptorType::eStorageBuffer,
                storageBuffers_[bufferIndex][frameIndex].getBuffer(),
                VK_WHOLE_SIZE, descriptorWrites, bufferInfos
            );
        }

        renderer->getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    return true;
}

bool DefaultVulkanMeshShaderPipeline::createQueryPool()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    vk::QueryPoolCreateInfo queryPoolInfo =
        vk::QueryPoolCreateInfo{}
            .setQueryType(vk::QueryType::ePipelineStatistics)
            .setQueryCount(1)
            .setPipelineStatistics(
                vk::QueryPipelineStatisticFlagBits(
                    vk::QueryPipelineStatisticFlagBits::
                        eTaskShaderInvocationsEXT
                )
            );

    queryPool_ = vk::raii::QueryPool(renderer->getDevice(), queryPoolInfo);

    return true;
}

void DefaultVulkanMeshShaderPipeline::addBufferWrite(
    vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
    vk::DescriptorType bufferType, vk::raii::Buffer *buffer,
    vk::DeviceSize bufferSize,
    std::vector<vk::WriteDescriptorSet> &descriptorWrites,
    std::vector<vk::DescriptorBufferInfo> &bufferInfos
)
{
    bufferInfos.emplace_back(
        vk::DescriptorBufferInfo{}.setBuffer(*buffer).setOffset(0).setRange(
            bufferSize
        )
    );
    descriptorWrites.emplace_back(
        vk::WriteDescriptorSet{}
            .setDstSet(descriptorSet)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(bufferType)
            .setPBufferInfo(&bufferInfos.back())
    );
}

bool DefaultVulkanMeshShaderPipeline::bind()
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

bool DefaultVulkanMeshShaderPipeline::update()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    vk::raii::CommandBuffer &commandBuffer = renderer->getCommandBuffer();
    bool startQuery = isDebugQuery_ && !isQuerying_;
    if (startQuery)
        commandBuffer.resetQueryPool(queryPool_, 0, 1);

    const DefaultMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const DefaultMeshShaderPipelineUpdateData &>(*updateData_);

    // scene data (uniform buffer index 0)
    SceneBuffer sceneData{};
    sceneData.view = *defaultUpdateData.camera->getViewMatrixRef();
    sceneData.projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(renderer->getSwapChainExtent().width) /
            static_cast<float>(renderer->getSwapChainExtent().height),
        0.1f, 10.0f
    );
    sceneData.projection[1][1] *= -1;
    sceneData.instanceCount = defaultUpdateData.instances->size();
    sceneData.meshletCount = geometry_->meshlets.size();
    memcpy(
        uniformBuffers_[0][renderer->getCurrentFrameIndex()].getMappedPointer(),
        &sceneData, sizeof(sceneData)
    );

    // instances data (storage buffer index 4)
    vk::DeviceSize instancesBufferSize =
        sizeof(glm::mat4x4) * defaultUpdateData.instances->size();
    storageBuffers_[4][renderer->getCurrentFrameIndex()].update(
        instancesBufferSize, defaultUpdateData.instances->data()
    );

    // debug data (storage buffer index 5)
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {DebugDataBuffer()};
    storageBuffers_[5][renderer->getCurrentFrameIndex()].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    return true;
}

bool DefaultVulkanMeshShaderPipeline::draw()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::raii::CommandBuffer &commandBuffer = renderer->getCommandBuffer();

    // TODO - wonder if you want to ensure that this is a nice number? eg 256,
    // etc
    // TODO - can't be calling this every frame
    const DefaultMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const DefaultMeshShaderPipelineUpdateData &>(*updateData_);
    uint32_t meshletCount = static_cast<uint32_t>(geometry_->meshlets.size());
    uint32_t instanceCount =
        static_cast<uint32_t>(defaultUpdateData.instances->size());
    uint32_t threadGroupCountX =
        ((meshletCount * instanceCount) / AS_GROUP_SIZE) + 1;

    bool startQuery = isDebugQuery_ && !isQuerying_;
    if (startQuery)
    {
        commandBuffer.beginQuery(queryPool_, 0);
        isQuerying_ = true;
    }

    renderer->getCommandBuffer().drawMeshTasksEXT(threadGroupCountX, 1, 1);

    if (startQuery)
        commandBuffer.endQuery(queryPool_, 0);

    return true;
}

bool DefaultVulkanMeshShaderPipeline::postRender()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    vk::DeviceSize debugDataBufferSize =
        sizeof(DebugDataBuffer) * debugDataBufferData_.size();
    storageBuffers_.back()[renderer->getCurrentFrameIndex()].copyToStaging(
        debugDataBufferSize
    );
    return true;
}

PipelineStatistics DefaultVulkanMeshShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool DefaultVulkanMeshShaderPipeline::updatePipelineStatistics()
{
    if (!isQuerying_)
        return false;

    // TODO - generalise
    vk::Result queryResult = queryPool_.getResults(
        0, 1, sizeof(uint64_t), pipelineStatisticsRaw_.data(), sizeof(uint64_t),
        vk::QueryResultFlagBits::e64
    );

    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );

    // TODO - this is a bit extreme
    renderer->waitAllFramesComplete();

    if (queryResult == vk::Result::eSuccess)
    {
        isQuerying_ = false;
        isDebugQuery_ = false;
        pipelineStatistics_.pipelineType =
            PipelineStatistics::PipelineType::MESH;

        vk::DeviceSize debugDataBufferSize =
            sizeof(DebugDataBuffer) * debugDataBufferData_.size();
        storageBuffers_.back()[renderer->getCurrentFrameIndex()].readData(
            debugDataBufferSize, debugDataBufferData_.data()
        );
        pipelineStatistics_.nTaskShaderInvocations =
            debugDataBufferData_[0].nTaskShaderInvocations;
        pipelineStatistics_.nMeshShaderInvocations =
            debugDataBufferData_[0].nMeshShaderInvocations;
        // pipelineStatistics_.nTaskShaderInvocations =
        // pipelineStatisticsRaw_[0];
        return true;
    }
    return false;
}
