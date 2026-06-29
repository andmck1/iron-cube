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
    vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eAll;

    protected:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory bufferMemory_ = nullptr;
    vk::raii::Buffer stagingBuffer_ = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory_ = nullptr;
};

class VulkanBufferIndex : public VulkanBufferBase
{
    public:
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
};

class VulkanBufferVertex : public VulkanBufferBase
{
    public:
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;
};

class VulkanBufferUniform : public VulkanBufferBase
{
    public:
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    void *getMappedPointer();

    protected:
    void *mappedPointer_ = nullptr;
};

class VulkanBufferStorageBase : public VulkanBufferBase
{
    public:
    void init(
        vk::DeviceSize bufferSize, void *data, vk::ShaderStageFlags stageFlags
    ) override;

    protected:
    const vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_;
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_;
};

class VulkanBufferRStorage : public VulkanBufferStorageBase
{
    protected:
    const vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
};

class VulkanBufferRWStorage : public VulkanBufferStorageBase
{
    public:
    void readData(vk::DeviceSize bufferSize, void *outputData);
    void copyToStaging(
        vk::DeviceSize size, vk::Flags<vk::PipelineStageFlagBits> pipelineStages
    );

    protected:
    const vk::Flags<vk::BufferUsageFlagBits> stagingBufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
    const vk::Flags<vk::BufferUsageFlagBits> bufferUsageFlags_ =
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eTransferSrc;
};
