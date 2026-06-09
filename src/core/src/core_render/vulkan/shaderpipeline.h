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

        auto [stagingBuffer, stagingBufferMemory] = renderer->createBuffer(
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

        void *stagingData = stagingBufferMemory.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);

        renderer->copyBuffer(stagingBuffer, buffer_, bufferSize);

        return true;
    }

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
};
