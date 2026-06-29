#include "core_render/vulkan/renderer.h"
#include "core_service/servicelocator.h"
#include <vulkan/vulkan_raii.hpp>

class VulkanBufferBase
{
    public:
    virtual void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) = 0;
    void update(vk::DeviceSize bufferSize, void *data);
    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
};

class VulkanBufferIndex : VulkanBufferBase
{
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
};

class VulkanBufferVertex : VulkanBufferBase
{
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
};

class VulkanBufferUniform : VulkanBufferBase
{
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    void *getMappedPointer();

    protected:
    void *mappedPointer_ = nullptr;
};

class VulkanBufferStorage : VulkanBufferBase
{
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
};

class VulkanBufferRWStorage : VulkanBufferBase
{
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    void readData(vk::DeviceSize bufferSize, void *outputData);
    void copyToStaging(
        vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages
    );
};

class VulkanBufferTEMP
{
    public:
    template <typename DataType>
    bool init(
        vk::DeviceSize bufferSize, DataType *data,
        vk::BufferUsageFlagBits bufferUsage, bool isHostVisible,
        bool isReadWrite = false
    )
    {
        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );

        if (isHostVisible)
        {
            std::tie(buffer_, bufferMemory_) = renderer->createBuffer(
                bufferSize, bufferUsage,
                vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent
            );
            mappedPointer_ = bufferMemory_.mapMemory(0, bufferSize);
            if (data)
            {
                memcpy(mappedPointer_, data, bufferSize);
            }
        }
        else
        {
            vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags;
            vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags;

            if (isReadWrite)
            {
                stagingBufferUsageFlags =
                    vk::BufferUsageFlagBits::eTransferDst |
                    vk::BufferUsageFlagBits::eTransferSrc;
                bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst |
                                   vk::BufferUsageFlagBits::eTransferSrc;
            }
            else
            {
                stagingBufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
                bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst;
            }

            std::tie(stagingBuffer_, stagingBufferMemory_) =
                renderer->createBuffer(
                    bufferSize, stagingBufferUsageFlags,
                    vk::MemoryPropertyFlagBits::eHostVisible |
                        vk::MemoryPropertyFlagBits::eHostCoherent
                );

            std::tie(buffer_, bufferMemory_) = renderer->createBuffer(
                bufferSize, bufferUsage | bufferUsageFlags,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

            if (data)
            {
                void *stagingData =
                    stagingBufferMemory_.mapMemory(0, bufferSize);
                memcpy(stagingData, data, bufferSize);
                stagingBufferMemory_.unmapMemory();
                renderer->copyBuffer(stagingBuffer_, buffer_, bufferSize);
            }
        }

        return true;
    }

    template <typename DataType>
    bool update(vk::DeviceSize bufferSize, DataType *data)
    {
        void *stagingData = stagingBufferMemory_.mapMemory(0, bufferSize);
        memcpy(stagingData, data, bufferSize);
        stagingBufferMemory_.unmapMemory();

        VulkanRenderer *renderer = dynamic_cast<VulkanRenderer *>(
            ServiceLocator::instance().get<IRenderer>()
        );
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

    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();
    void *getMappedPointer();
    // TODO - pipeline stages is part of the buffer's state and shouldn't
    // need to be passed in
    void copyToStaging(
        vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages
    );

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
    void *mappedPointer_ = nullptr;
};
