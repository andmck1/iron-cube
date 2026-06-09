#include "defaultMesh.h"
#include "core_buffertypes/buffertypes.h"
#include "core_ecs/component/transform.h"
#include "core_render/ishadermanager.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/shader.h"
#include "core_service/servicelocator.h"
#include "glm/ext/matrix_clip_space.hpp"

bool DefaultVulkanMeshShaderPipeline::init(
    Geometry *geometry, ShaderPipelineUpdateData *updateData
)
{
    updateData_ = updateData;
    return initPipeline(geometry);
}

// TODO - you can have the vertex and index buffers in one buffer and use
// offsets (see tutorial on index buffer)
// TODO - you can have a separate command pool for transfer instructions (see
// same tutorial)
//
bool DefaultVulkanMeshShaderPipeline::initPipeline(Geometry *geometry)
{
    geometry_ = geometry;
    if (!initDescriptorLayout())
        return false;
    if (!initUniformBuffers())
        return false;
    if (!initStorageBuffers())
        return false;
    if (!initDescriptorSets())
        return false;
    if (!initGraphicsPipeline())
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
    vertexBuffer_ = VulkanBuffer();
    meshletBuffer_ = VulkanBuffer();
    meshletVerticesBuffer_ = VulkanBuffer();
    meshletTrianglesBuffer_ = VulkanBuffer();

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
    VulkanShader *defaultMeshShader =
        dynamic_cast<VulkanShader *>(shaderManager->getShader("defaultMesh"));
    VulkanShader *defaultFragmentShader = dynamic_cast<VulkanShader *>(
        shaderManager->getShader("defaultFragment")
    );

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

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        meshShaderStageInfo, fragShaderStageInfo
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
            .setStageCount(2)
            .setPStages(shaderStages)
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

    vk::DescriptorSetLayoutBinding mvpLayoutBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT);

    vk::DescriptorSetLayoutBinding vertexBufferBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(1)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT);

    vk::DescriptorSetLayoutBinding meshletBufferBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(2)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT);

    vk::DescriptorSetLayoutBinding meshletVerticesBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(3)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT);

    vk::DescriptorSetLayoutBinding meshletTrianglesBinding =
        vk::DescriptorSetLayoutBinding{}
            .setBinding(4)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eMeshEXT);

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        mvpLayoutBinding, vertexBufferBinding, meshletBufferBinding,
        meshletVerticesBinding, meshletTrianglesBinding
    };

    vk::DescriptorSetLayoutCreateInfo layoutInfo =
        vk::DescriptorSetLayoutCreateInfo{}
            .setBindingCount(bindings.size())
            .setPBindings(bindings.data());

    descriptorSetLayout_ =
        vk::raii::DescriptorSetLayout(renderer->getDevice(), layoutInfo);
    return true;
}

bool DefaultVulkanMeshShaderPipeline::initUniformBuffers()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DeviceSize bufferSize = sizeof(MVPBuffer);
        auto [buffer, bufferMem] = renderer->createBuffer(
            bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent
        );
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemory.emplace_back(std::move(bufferMem));
        uniformBuffersMapped.emplace_back(
            uniformBuffersMemory.back().mapMemory(0, bufferSize)
        );
    }

    return true;
}

bool DefaultVulkanMeshShaderPipeline::initStorageBuffers()
{
    vk::DeviceSize vertexBufferSize =
        sizeof(VertexInputType) * geometry_->vertices.size();
    vk::DeviceSize meshletBufferSize =
        sizeof(meshopt_Meshlet) * geometry_->meshlets.size();
    vk::DeviceSize meshletVerticesBufferSize =
        sizeof(uint32_t) * geometry_->meshletVertices.size();
    vk::DeviceSize meshletTrianglesBufferSize =
        sizeof(uint32_t) * geometry_->meshletTrianglesU32.size();
    //
    vertexBuffer_.init(vertexBufferSize, geometry_->vertices.data());
    meshletBuffer_.init(meshletBufferSize, geometry_->meshlets.data());
    meshletVerticesBuffer_.init(
        meshletVerticesBufferSize, geometry_->meshletVertices.data()
    );
    meshletTrianglesBuffer_.init(
        meshletTrianglesBufferSize, geometry_->meshletTrianglesU32.data()
    );
    return true;
}

bool DefaultVulkanMeshShaderPipeline::initDescriptorSets()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    std::vector<vk::DescriptorSetLayout> layouts(
        renderer->MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout_
    );
    vk::DescriptorSetAllocateInfo allocInfo =
        vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool(renderer->getDescriptorPool())
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

    descriptorSets_ = renderer->getDevice().allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < renderer->MAX_FRAMES_IN_FLIGHT; i++)
    {
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        descriptorWrites.reserve(5);
        bufferInfos.reserve(5);

        addUniformBufferWrite(
            descriptorSets_[i], 0, &uniformBuffers[i], sizeof(MVPBuffer),
            descriptorWrites, bufferInfos
        );
        addStorageBufferWrite(
            descriptorSets_[i], 1, &vertexBuffer_, descriptorWrites, bufferInfos
        );
        addStorageBufferWrite(
            descriptorSets_[i], 2, &meshletBuffer_, descriptorWrites,
            bufferInfos
        );
        addStorageBufferWrite(
            descriptorSets_[i], 3, &meshletVerticesBuffer_, descriptorWrites,
            bufferInfos
        );
        addStorageBufferWrite(
            descriptorSets_[i], 4, &meshletTrianglesBuffer_, descriptorWrites,
            bufferInfos
        );

        renderer->getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    return true;
}

void DefaultVulkanMeshShaderPipeline::addUniformBufferWrite(
    vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
    vk::raii::Buffer *buffer, vk::DeviceSize range,
    std::vector<vk::WriteDescriptorSet> &descriptorWrites,
    std::vector<vk::DescriptorBufferInfo> &bufferInfos
)
{
    bufferInfos.emplace_back(
        vk::DescriptorBufferInfo{}.setBuffer(*buffer).setOffset(0).setRange(
            range
        )
    );
    descriptorWrites.emplace_back(
        vk::WriteDescriptorSet{}
            .setDstSet(descriptorSet)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo(&bufferInfos.back())
    );
}

void DefaultVulkanMeshShaderPipeline::addStorageBufferWrite(
    vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
    VulkanBuffer *vulkanBuffer,
    std::vector<vk::WriteDescriptorSet> &descriptorWrites,
    std::vector<vk::DescriptorBufferInfo> &bufferInfos
)
{
    bufferInfos.emplace_back(
        vk::DescriptorBufferInfo{}
            .setBuffer(*vulkanBuffer->getBuffer())
            .setOffset(0)
            .setRange(VK_WHOLE_SIZE)
    );
    descriptorWrites.emplace_back(
        vk::WriteDescriptorSet{}
            .setDstSet(descriptorSet)
            .setDstBinding(binding)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
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

    const DefaultMeshShaderPipelineUpdateData &defaultUpdateData =
        dynamic_cast<const DefaultMeshShaderPipelineUpdateData &>(*updateData_);

    // MVP data
    MVPBuffer mvp{};
    mvp.model = *defaultUpdateData.actor->getComponent<TransformComponent>()
                     ->getTransformMatrixRef();
    mvp.view = *defaultUpdateData.camera->getViewMatrixRef();
    mvp.projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(renderer->getSwapChainExtent().width) /
            static_cast<float>(renderer->getSwapChainExtent().height),
        0.1f, 10.0f
    );
    mvp.projection[1][1] *= -1;
    memcpy(
        uniformBuffersMapped[renderer->getCurrentFrameIndex()], &mvp,
        sizeof(mvp)
    );

    return true;
}

bool DefaultVulkanMeshShaderPipeline::draw()
{
    VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    renderer->getCommandBuffer().drawMeshTasksEXT(
        geometry_->meshlets.size(), 1, 1
    );
    return true;
}
