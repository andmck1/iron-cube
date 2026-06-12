#pragma once

#include "core_actor/camera.h"
#include "core_actor/sceneactor.h"
#include "core_common/types.h"
#include "core_render/vulkan/shaderpipeline.h"
#include <vulkan/vulkan_raii.hpp>

class DefaultMeshShaderPipelineUpdateData : public ShaderPipelineUpdateData
{
    public:
    SceneActor *actor = nullptr;
    CameraActor *camera = nullptr;
    std::vector<Instance> *instances = nullptr;
};

class VulkanUniformBuffer
{
    public:
    bool init(
        vk::DeviceSize bufferSize, vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties
    );
    vk::raii::Buffer *getBuffer();
    void *getMappedPointer();

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    void *mappedPointer_ = nullptr;
};

class DefaultVulkanMeshShaderPipeline : public VulkanShaderPipeline
{
    public:
    bool
    init(Geometry *geometry, ShaderPipelineUpdateData *updateData) override;
    bool update() override;
    bool bind() override;
    bool shutdown() override;
    bool recreateGraphicsPipeline() override;
    bool draw() override;

    protected:
    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    VulkanBuffer vertexBuffer_;
    VulkanBuffer meshletBuffer_;
    VulkanBuffer meshletVerticesBuffer_;
    VulkanBuffer meshletTrianglesBuffer_;
    VulkanBuffer instancesBuffer_;

    std::vector<VulkanUniformBuffer> sceneBuffers_;

    Geometry *geometry_ = nullptr;

    bool initGraphicsPipeline();
    bool initPipeline(Geometry *geometry);
    bool initDescriptorLayout();
    bool initUniformBuffers();
    bool initStorageBuffers();
    bool initDescriptorSets();
    bool createDescriptorPool();

    void addUniformBufferWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        vk::raii::Buffer *buffer, vk::DeviceSize range,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );
    void addStorageBufferWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        VulkanBuffer *vulkanBuffer,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );
};
