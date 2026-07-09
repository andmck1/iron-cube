#pragma once

#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan_raii.hpp>

struct BufferDescription
{
    uint bufferSlot;
    vk::DeviceSize bufferSize;
    void *data;

    vk::DescriptorType descriptorType;
    vk::BufferUsageFlags deviceUsage;
    vk::BufferUsageFlags stagingUsage;
    vk::MemoryPropertyFlags deviceMemory;
    vk::MemoryPropertyFlags stagingMemoryProperties;
    vk::ShaderStageFlags stageFlags;
    bool useStaging = false;
    bool persistentlyMapped = false;
    bool readable = false;
};

class VulkanBuffer
{
    public:
    void init(BufferDescription bufferDescription);
    void update(vk::DeviceSize bufferSize, void *data);
    vk::raii::Buffer *getBuffer(uint32_t frameIndex);
    vk::raii::DeviceMemory *getDeviceMemory(uint32_t frameIndex);
    void readData(vk::DeviceSize bufferSize, void *outputData);
    void copyToStaging(
        vk::DeviceSize size,
        vk::Flags<vk::PipelineStageFlagBits> pipelineStages, uint32_t frameIndex
    );
    void *getMappedPointer(uint32_t frameIndex);

    BufferDescription description;

    protected:
    void copyBuffer(
        vk::raii::Buffer &fromBuffer, vk::raii::Buffer &toBuffer,
        vk::raii::DeviceMemory &fromBufferMemory, vk::DeviceSize bufferSize,
        void *data
    );

    struct FrameBuffer
    {
        vk::raii::Buffer buffer = nullptr;
        vk::raii::DeviceMemory memory = nullptr;
        void *mappedPointer = nullptr;
    };

    std::vector<FrameBuffer> frames_;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;
    vk::DeviceSize bufferSize_ = 0;
};

// Predefined buffer descriptions for common buffer types
extern BufferDescription UniformBuffer;
extern BufferDescription StorageRead;
extern BufferDescription StorageReadWrite;
extern BufferDescription VertexBuffer;
extern BufferDescription IndexBuffer;
