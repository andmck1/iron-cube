#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan_raii.hpp>

class VulkanBufferBase
{
    public:
    void update(vk::DeviceSize bufferSize, void *data);
    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();

    protected:
    void copyBuffer(
        vk::raii::Buffer &fromBuffer, vk::raii::Buffer &toBuffer,
        vk::raii::DeviceMemory &fromBufferMemory, vk::DeviceSize bufferSize,
        void *data
    );

    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;
};

class VulkanBufferSingleStageBase : public VulkanBufferBase
{
    public:
    VulkanBufferSingleStageBase(vk::Flags<vk::BufferUsageFlagBits> bufferFlags)
        : bufferUsageFlags_(bufferFlags)
    {
    }
    void init(vk::DeviceSize bufferSize, void *data);

    protected:
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_;
};

class VulkanBufferIndex : public VulkanBufferSingleStageBase
{
    public:
    VulkanBufferIndex()
        : VulkanBufferSingleStageBase(vk::BufferUsageFlagBits::eIndexBuffer)
    {
    }
};

class VulkanBufferVertex : public VulkanBufferSingleStageBase
{
    public:
    VulkanBufferVertex()
        : VulkanBufferSingleStageBase(
              vk::BufferUsageFlagBits::eVertexBuffer |
              vk::BufferUsageFlagBits::eTransferDst
          )
    {
    }
};

class VulkanBufferMultiStageBase : public VulkanBufferBase
{
    public:
    virtual void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) = 0;
    vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll;
};

class VulkanBufferUniform : public VulkanBufferMultiStageBase
{
    public:
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
    void update(vk::DeviceSize bufferSize, void *data);

    void *getMappedPointer();

    protected:
    void *mappedPointer_ = nullptr;
};

class VulkanBufferStorageBase : public VulkanBufferMultiStageBase
{
    public:
    VulkanBufferStorageBase(
        vk::Flags<vk::BufferUsageFlagBits> stagingFlags,
        vk::Flags<vk::BufferUsageFlagBits> bufferFlags
    )
        : stagingBufferUsageFlags_(stagingFlags), bufferUsageFlags_(bufferFlags)
    {
    }
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    protected:
    const vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_;
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_;
};

class VulkanBufferRStorage : public VulkanBufferStorageBase
{
    public:
    VulkanBufferRStorage()
        : VulkanBufferStorageBase(
              vk::BufferUsageFlagBits::eTransferSrc,
              vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eStorageBuffer
          )
    {
    }

    protected:
    vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferSrc;
    vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst;
};

class VulkanBufferRWStorage : public VulkanBufferStorageBase
{
    public:
    VulkanBufferRWStorage()
        : VulkanBufferStorageBase(
              vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc,
              vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc |
                  vk::BufferUsageFlagBits::eStorageBuffer
          )
    {
    }
    void readData(vk::DeviceSize bufferSize, void *outputData);
    void copyToStaging(
        vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages
    );

    protected:
    vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
    vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
};

class VulkanBufferIndirectStorage : public VulkanBufferStorageBase
{
    public:
    VulkanBufferIndirectStorage()
        : VulkanBufferStorageBase(
              vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc,
              vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc |
                  vk::BufferUsageFlagBits::eStorageBuffer |
                  vk::BufferUsageFlagBits::eIndirectBuffer
          )
    {
    }
};
