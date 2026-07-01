#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan_raii.hpp>

class VulkanBufferBase
{
    public:
    void update(vk::DeviceSize bufferSize, void *data);
    vk::raii::Buffer *getBuffer();
    vk::raii::DeviceMemory *getDeviceMemory();

    protected:
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
        : VulkanBufferSingleStageBase(vk::BufferUsageFlagBits::eVertexBuffer)
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

    void *getMappedPointer();

    protected:
    void *mappedPointer_ = nullptr;
};

class VulkanBufferDynamicStorageBase : public VulkanBufferMultiStageBase
{
    public:
    VulkanBufferDynamicStorageBase(
        vk::Flags<vk::BufferUsageFlagBits> stagingFlags,
        vk::Flags<vk::BufferUsageFlagBits> bufferFlags
    )
        : stagingBufferUsageFlags_(stagingFlags), bufferUsageFlags_(bufferFlags)
    {
    }
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    void update(vk::DeviceSize bufferSize, void *data);

    void *getMappedPointer();

    protected:
    void *mappedPointer_ = nullptr;
    const vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_;
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_;
};

class VulkanBufferStaticStorageBase : public VulkanBufferMultiStageBase
{
    public:
    VulkanBufferStaticStorageBase(
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

class VulkanBufferStaticRStorage : public VulkanBufferStaticStorageBase
{
    public:
    VulkanBufferStaticRStorage()
        : VulkanBufferStaticStorageBase(
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

class VulkanBufferStaticRWStorage : public VulkanBufferStaticStorageBase
{
    public:
    VulkanBufferStaticRWStorage()
        : VulkanBufferStaticStorageBase(
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

class VulkanBufferDynamicRStorage : public VulkanBufferDynamicStorageBase
{
    public:
    VulkanBufferDynamicRStorage()
        : VulkanBufferDynamicStorageBase(
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

class VulkanBufferDynamicRWStorage : public VulkanBufferDynamicStorageBase
{
    public:
    VulkanBufferDynamicRWStorage()
        : VulkanBufferDynamicStorageBase(
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
