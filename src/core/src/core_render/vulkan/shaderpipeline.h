#pragma once

#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/pipelineresource.h"
#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

struct BufferBarrierInfo
{
    vk::Flags<vk::AccessFlagBits2> sourceAccess;
    vk::Flags<vk::AccessFlagBits2> destinationAccess;
    vk::Flags<vk::PipelineStageFlagBits2> sourceStage;
    vk::Flags<vk::PipelineStageFlagBits2> destinationStage;
    vk::raii::Buffer *buffer;
};

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    void addResource(const ResourceDescription &description);

    void addResourceWrite(
        vk::raii::DescriptorSet &descriptorSet, uint32_t binding,
        vk::DescriptorType bufferType,
        std::variant<vk::raii::Buffer *, vk::raii::Image *> buffer,
        vk::DeviceSize bufferSize,
        std::vector<vk::WriteDescriptorSet> &descriptorWrites,
        std::vector<vk::DescriptorBufferInfo> &bufferInfos
    );

    bool initDescriptorLayout();
    bool initDescriptorSets();
    bool createDescriptorPool();
    void shutdownBuffers();

    virtual void initBufferDescriptions() = 0;
    virtual void initBuffers() = 0;

    void addBarrier(BufferBarrierInfo barrierInfo);
    void setBarrier();

    std::optional<vk::raii::Pipeline> graphicsPipeline_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;

    vk::raii::DescriptorPool descriptorPool_ = nullptr;
    vk::raii::DescriptorSets descriptorSets_ = nullptr;
    vk::raii::DescriptorSetLayout descriptorSetLayout_ = nullptr;
    vk::raii::PipelineLayout pipelineLayout_ = nullptr;

    std::vector<ResourceDescription> resourceDescriptions_;
    std::vector<PipelineResource> resources_;
    std::vector<vk::BufferMemoryBarrier2> barriers_;
};
