#include "shaderpipeline/instancedVertex.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "helpers/globals.h"

bool InstancedVertexVulkanShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    updateData_ =
        dynamic_cast<InstancedVertexShaderPipelineUpdateData *>(updateData);
    geometry_ = geometry;
    return true;
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool InstancedVertexVulkanShaderPipeline::initPipeline()
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

bool InstancedVertexVulkanShaderPipeline::recreateGraphicsPipeline()
{
    spdlog::debug("Recreating graphics pipeline");
    graphicsPipeline_ = nullptr;
    return initGraphicsPipeline();
}

bool InstancedVertexVulkanShaderPipeline::shutdown()
{
    spdlog::debug("Shutting down instanced vulkan pipeline");
    descriptorSetLayout_ = nullptr;
    descriptorSets_ = nullptr;
    descriptorPool_ = nullptr;
    pipelineLayout_ = nullptr;
    graphicsPipeline_ = nullptr;
    shutdownBuffers();
    spdlog::debug("Shutdown complete for instanced vulkan pipeline");
    return true;
}

void InstancedVertexVulkanShaderPipeline::initBufferDescriptions()
{
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::MVP)
            .setBufferSize(sizeof(InstancedVF::MVPBuffer))
            .setData(static_cast<InstancedVF::MVPBuffer *>(nullptr))
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        UniformBuffer.setBufferSlot(InstancedVF::BufferSlots::ANIMATION)
            .setBufferSize(sizeof(InstancedVF::AnimationBuffer))
            .setData(static_cast<InstancedVF::AnimationBuffer *>(nullptr))
            .setStageFlags(vk::ShaderStageFlagBits::eVertex)
    );
    resourceDescriptions_.push_back(
        StorageReadWrite.setBufferSlot(InstancedVF::BufferSlots::DEBUG)
            .setBufferSize(sizeof(InstancedVF::DebugDataBuffer))
            .setData(debugDataBufferData_.data())
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
    );
    resourceDescriptions_.push_back(
        ImageDescription()
            .setBufferSlot(InstancedVF::BufferSlots::TEXTURE)
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

void InstancedVertexVulkanShaderPipeline::initBuffers()
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
    instanceDesc.bufferSize = sizeof(Instance) * MAX_N_INSTANCES;
    instanceDesc.data = nullptr;
    instanceBuffer_.init(instanceDesc);
}

bool InstancedVertexVulkanShaderPipeline::initGraphicsPipeline()
{

    IShaderManager *shaderManager =
        ServiceLocator::instance().get<IShaderManager>();
    VulkanShader *instancedVertexShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFVertex")
    );
    VulkanShader *instancedFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("instancedVFFragment")
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

    std::vector<vk::PushConstantRange> pushConstantRanges = {
        vk::PushConstantRange{}
            .setStageFlags(
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eFragment
            )
            .setOffset(0)
            .setSize(sizeof(InstancedVF::Constants)),
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

bool InstancedVertexVulkanShaderPipeline::bind()
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

void InstancedVertexVulkanShaderPipeline::updateMVPBuffer()
{
    InstancedVF::MVPBuffer mvp{};
    mvp.model = *updateData_->actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *updateData_->camera->getViewMatrixRef();
    mvp.projection = *updateData_->camera->getProjectionMatrixRef();
    resources_[InstancedVF::BufferSlots::MVP].update(sizeof(mvp), &mvp);
}

void InstancedVertexVulkanShaderPipeline::updateAnimationBuffer()
{
    InstancedVF::AnimationBuffer animation{};
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
    resources_[InstancedVF::BufferSlots::ANIMATION].update(
        sizeof(animation), &animation
    );
}

void InstancedVertexVulkanShaderPipeline::updateInstancesBuffer()
{
    std::array<std::vector<Instance>, MAX_LOD_COUNT> lodInstances;
    std::vector<Instance> sortedInstances;
    sortedInstances.reserve(updateData_->instances->size());
    lodCounts_.fill(0);
    lodCumulativeCounts_.fill(0);

    vk::DeviceSize instanceBufferSize =
        sizeof(Instance) * updateData_->instances->size();

    if (*updateData_->isLODOn)
    {

        glm::vec4 minMeshBounds = {geometry_->meshBounds.minCorner, 1.0f};
        glm::vec4 maxMeshBounds = {geometry_->meshBounds.maxCorner, 1.0f};
        glm::vec4 eyePosition = {
            updateData_->camera->getComponent<TransformComponent>()
                ->getPosition(),
            1.0f
        };
        float maxLODDistance = *updateData_->maxLODDistance;
        float lodExponent = *updateData_->lodExponent;
        for (const Instance &instance : *updateData_->instances)
        {
            glm::mat4x4 worldMatrix = calculateModelMatrix(
                {instance.position, 1.f}, {instance.rotation, 1.f},
                {instance.scale, 1.f}
            );
            uint lodLevel = calculateLOD(
                worldMatrix, minMeshBounds, maxMeshBounds, eyePosition,
                maxLODDistance, lodExponent, MAX_LOD_COUNT
            );

            lodInstances[lodLevel].push_back(instance);
            lodCounts_[lodLevel]++;
        }
        for (uint lodLevel = 0; lodLevel < MAX_LOD_COUNT; lodLevel++)
        {
            sortedInstances.insert(
                sortedInstances.end(), lodInstances[lodLevel].begin(),
                lodInstances[lodLevel].end()
            );
            lodCumulativeCounts_[lodLevel] +=
                lodLevel > 0 ? lodCumulativeCounts_[lodLevel - 1] +
                                   lodCounts_[lodLevel - 1]
                             : 0;
        }

        instanceBuffer_.update(instanceBufferSize, sortedInstances.data());
    }
    else
    {
        instanceBuffer_.update(
            instanceBufferSize, updateData_->instances->data()
        );
    }
}

void InstancedVertexVulkanShaderPipeline::updateDebugBuffer()
{
    updatePipelineStatistics();
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    debugDataBufferData_ = {InstancedVF::DebugDataBuffer()};
    debugDataBufferData_[0].isDebugQueryOn = isDebugQuery_;
    resources_[InstancedVF::BufferSlots::DEBUG].update(
        debugDataBufferSize, debugDataBufferData_.data()
    );
}

void InstancedVertexVulkanShaderPipeline::updateConstantsBuffer(uint lodLevel)
{
    InstancedVF::Constants constants = InstancedVF::Constants();
    constants.fragmentColourType = *updateData_->fragmentColourType;
    constants.lodLevel = lodLevel;
    renderer_->getCommandBuffer().pushConstants(
        pipelineLayout_,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0, sizeof(InstancedVF::Constants), &constants
    );
}

bool InstancedVertexVulkanShaderPipeline::update()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();

    updateMVPBuffer();
    updateAnimationBuffer();
    updateInstancesBuffer();
    updateDebugBuffer();

    updateTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool InstancedVertexVulkanShaderPipeline::draw()
{
    std::chrono::time_point now = std::chrono::steady_clock::now();
    nDrawCalls_ = 0;

    if (*updateData_->isLODOn)
    {
        for (uint lodLevel = 0; lodLevel < MAX_LOD_COUNT; lodLevel++)
        {
            uint32_t lodCount = lodCounts_[lodLevel];
            if (lodCount > 0)
            {
                updateConstantsBuffer(lodLevel);
                uint32_t nIndices = geometry_->indexCounts[lodLevel];
                uint32_t indexOffset = geometry_->indexOffsets[lodLevel];
                renderer_->getCommandBuffer().drawIndexed(
                    nIndices, lodCount, indexOffset, 0,
                    lodCumulativeCounts_[lodLevel]
                );
                nDrawCalls_++;
            }
        }
    }
    else
    {
        updateConstantsBuffer(0);
        uint32_t nIndices = geometry_->indexCounts[0];
        uint32_t indexOffset = geometry_->indexOffsets[0];
        renderer_->getCommandBuffer().drawIndexed(
            nIndices, updateData_->instances->size(), indexOffset, 0, 0
        );
    }

    drawTime =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - now)
            .count();

    return true;
}

bool InstancedVertexVulkanShaderPipeline::postRender()
{
    if (!isDebugQuery_)
        return true;
    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[InstancedVF::BufferSlots::DEBUG].asBuffer()->copyToStaging(
        debugDataBufferSize,
        vk::PipelineStageFlagBits::eVertexShader |
            vk::PipelineStageFlagBits::eFragmentShader,
        renderer_->getCurrentFrameIndex()
    );
    return true;
}

PipelineStatistics InstancedVertexVulkanShaderPipeline::getPipelineStatistics()
{
    return pipelineStatistics_;
}

bool InstancedVertexVulkanShaderPipeline::updatePipelineStatistics()
{
    if (!isDebugQuery_)
        return false;

    vk::DeviceSize debugDataBufferSize =
        sizeof(InstancedVF::DebugDataBuffer) * debugDataBufferData_.size();
    resources_[InstancedVF::BufferSlots::DEBUG].asBuffer()->readData(
        debugDataBufferSize, debugDataBufferData_.data()
    );

    long nInstances = updateData_->instances->size();
    pipelineStatistics_.stats = {
        {"N Instances", nInstances},
        {
            "N Vertices",
            geometry_->vertices.size() * nInstances,
        },
        {
            "N Triangles",
            geometry_->vertices.size() / 3 * nInstances,
        },
        {"N VS Invocations", debugDataBufferData_[0].nVertexShaderInvocations},
        {"N FS Invocations",
         debugDataBufferData_[0].nFragmentShaderInvocations},
        {"N Draw Calls", nDrawCalls_},
    };

    return true;
}

uint InstancedVertexVulkanShaderPipeline::calculateLOD(
    glm::mat4x4 modelMatrix, glm::vec4 minMeshBounds, glm::vec4 maxMeshBounds,
    glm::vec3 eyePosition, float maxLODDistance, float lodExponent,
    uint maxLODCount
)
{
    float4 instanceBoundsMinWS = modelMatrix * minMeshBounds;
    float4 instanceBoundsMaxWS = modelMatrix * maxMeshBounds;
    float4 instanceCentre = (instanceBoundsMinWS + instanceBoundsMaxWS) / 2.f;
    float distanceToMesh =
        glm::distance(glm::vec3(instanceCentre), eyePosition);
    float normalisedDistanceToMesh =
        std::clamp(distanceToMesh / maxLODDistance, 0.f, 1.f);
    uint lod =
        (uint) (pow(normalisedDistanceToMesh, lodExponent) * (maxLODCount - 1));
    return lod;
}

glm::mat4x4 InstancedVertexVulkanShaderPipeline::calculateModelMatrix(
    float4 transform, float4 rotation, float4 scale
)
{
    float cosX = cos(rotation.x);
    float cosY = cos(rotation.y);
    float cosZ = cos(rotation.z);

    float sinX = sin(rotation.x);
    float sinY = sin(rotation.y);
    float sinZ = sin(rotation.z);

    return float4x4(
        // row1
        scale.x * (cosY * cosZ), scale.y * (cosY * sinZ), scale.z * (-sinY),
        transform.x,
        // row 2
        scale.x * (sinX * sinY * cosZ - cosX * sinZ),
        scale.y * (sinX * sinY * sinZ + cosX * cosZ), scale.z * (sinX * cosY),
        transform.y,
        // row 3
        scale.x * (cosX * sinY * cosZ + sinX * sinZ),
        scale.y * (cosX * sinY * sinZ - sinX * cosZ), scale.z * (cosX * cosY),
        transform.z,
        // row 4
        0.0, 0.0, 0.0, 1.0
    );
}
