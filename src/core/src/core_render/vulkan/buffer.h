#pragma once

#include "shader/commontypes.h"
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/stagingtransfer.h"
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

    BufferDescription &setBufferSlot(uint slot)
    {
        bufferSlot = slot;
        return *this;
    }

    BufferDescription &setBufferSize(vk::DeviceSize size)
    {
        bufferSize = size;
        return *this;
    }

    BufferDescription &setData(void *dataPtr)
    {
        data = dataPtr;
        return *this;
    }

    BufferDescription &setDescriptorType(vk::DescriptorType type)
    {
        descriptorType = type;
        return *this;
    }

    BufferDescription &setDeviceUsage(vk::BufferUsageFlags usage)
    {
        deviceUsage = usage;
        return *this;
    }

    BufferDescription &setStagingUsage(vk::BufferUsageFlags usage)
    {
        stagingUsage = usage;
        return *this;
    }

    BufferDescription &setDeviceMemory(vk::MemoryPropertyFlags memory)
    {
        deviceMemory = memory;
        return *this;
    }

    BufferDescription &setStagingMemoryProperties(vk::MemoryPropertyFlags props)
    {
        stagingMemoryProperties = props;
        return *this;
    }

    BufferDescription &setStageFlags(vk::ShaderStageFlags flags)
    {
        stageFlags = flags;
        return *this;
    }

    BufferDescription &setUseStaging(bool use)
    {
        useStaging = use;
        return *this;
    }

    BufferDescription &setPersistentlyMapped(bool mapped)
    {
        persistentlyMapped = mapped;
        return *this;
    }

    BufferDescription &setReadable(bool readableFlag)
    {
        readable = readableFlag;
        return *this;
    }
};

struct FrameBuffer
{
    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    void *mappedPointer = nullptr;
    StagingTransfer staging;
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
    FrameBuffer *getFrameBuffer(uint32_t frameIndex);

    BufferDescription description;

    protected:
    void copyBuffer(
        vk::raii::Buffer &fromBuffer, vk::raii::Buffer &toBuffer,
        vk::raii::DeviceMemory &fromBufferMemory, vk::DeviceSize bufferSize,
        void *data
    );

    std::vector<FrameBuffer> frames_;
    VulkanRenderer *renderer_ = nullptr;
};

extern BufferDescription UniformBuffer;
extern BufferDescription StorageRead;
extern BufferDescription StorageReadWrite;
extern BufferDescription VertexBuffer;
extern BufferDescription IndexBuffer;
