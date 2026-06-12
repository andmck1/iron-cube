#pragma once

#include "core_render/ishaderpipeline.h"
#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <vulkan/vulkan_raii.hpp>

class VulkanShaderPipeline : public IShaderPipeline
{
    public:
    std::optional<vk::raii::Pipeline> &getGraphicsPipeline();

    protected:
    std::optional<vk::raii::Pipeline> graphicsPipeline_;
    ShaderPipelineUpdateData *updateData_ = nullptr;
};

class VulkanBuffer
{
    public:
    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();

    template <typename DataType>
    bool init(vk::DeviceSize bufferSize, DataType *data)
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );

        std::tie(stagingBuffer_, stagingBufferMemory_) = renderer->createBuffer(
            bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent
        );

        std::tie(buffer_, bufferMemory_) = renderer->createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eStorageBuffer |
                vk::BufferUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal
        );

        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();

        renderer->copyBuffer(stagingBuffer_, buffer_, bufferSize);

        return true;
    }

    template <typename DataType>
    bool update(vk::DeviceSize bufferSize, DataType *data)
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );

        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();

        renderer->copyBuffer(stagingBuffer_, buffer_, bufferSize);

        return true;
    }

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
};
