#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <vulkan/vulkan_raii.hpp>

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

class VulkanBuffer
{
    public:
    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();
    void copyToStaging(vk::DeviceSize size);

    template <typename DataType>
    bool init(
        vk::DeviceSize bufferSize, DataType *data,
        vk::BufferUsageFlagBits bufferType, bool isReadWrite
    )
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );

        vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags;
        vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags;

        if (isReadWrite)
        {
            stagingBufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst |
                                      vk::BufferUsageFlagBits::eTransferSrc;
            bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst |
                               vk::BufferUsageFlagBits::eTransferSrc;
        }
        else
        {
            stagingBufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
            bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst;
        }

        std::tie(stagingBuffer_, stagingBufferMemory_) = renderer->createBuffer(
            bufferSize, stagingBufferUsageFlags,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent
        );

        std::tie(buffer_, bufferMemory_) = renderer->createBuffer(
            bufferSize, bufferType | bufferUsageFlags,
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

    template <typename DataType>
    bool readData(vk::DeviceSize bufferSize, DataType *outputData)
    {
        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(outputData, stagingData, bufferSize);
        stagingBufferMemory_.unmapMemory();

        return true;
    }

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
};
