// AI help here

#pragma once
#include "core_render/vulkan/buffer.h"
#include "core_render/vulkan/image.h"
#include <variant>

using ResourceDescription = std::variant<BufferDescription, ImageDescription>;

class PipelineResource
{
    public:
    void init(const ResourceDescription &desc);
    uint getBufferSlot() const;
    vk::DescriptorType getDescriptorType() const;
    vk::ShaderStageFlags getStageFlags() const;
    VulkanBuffer *asBuffer();
    VulkanImage *asImage();
    void update(vk::DeviceSize bufferSize, void *data);

    private:
    std::variant<std::monostate, VulkanBuffer, VulkanImage> resource_;
};
