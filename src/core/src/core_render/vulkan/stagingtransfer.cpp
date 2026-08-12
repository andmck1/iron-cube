// AI help here

#include "core_render/vulkan/stagingtransfer.h"
#include <cstring>

void StagingTransfer::init(VulkanRenderer *renderer, vk::DeviceSize size)
{
    renderer_ = renderer;
    size_ = size;
    std::tie(buffer_, memory_) = renderer_->createBuffer(
        size,
        vk::BufferUsageFlagBits::eTransferSrc |
            vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent
    );
}

bool StagingTransfer::isInitialized() const
{
    return static_cast<bool>(*buffer_);
}

void StagingTransfer::upload(void *data, vk::DeviceSize size)
{
    void *mapped = memory_.mapMemory(0, size);
    memcpy(mapped, data, size);
    memory_.unmapMemory();
}

void StagingTransfer::download(void *outData, vk::DeviceSize size)
{
    void *mapped = memory_.mapMemory(0, size);
    memcpy(outData, mapped, size);
    memory_.unmapMemory();
}

vk::raii::Buffer &StagingTransfer::buffer()
{
    return buffer_;
}
vk::DeviceSize StagingTransfer::size() const
{
    return size_;
}
