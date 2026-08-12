// AI help here

#include "core_render/vulkan/pipelineresource.h"

void PipelineResource::init(const ResourceDescription &desc)
{
    std::visit(
        [this](auto &&d)
        {
            using T = std::decay_t<decltype(d)>;
            if constexpr (std::is_same_v<T, BufferDescription>)
            {
                auto &buf = resource_.emplace<VulkanBuffer>();
                buf.init(d);
            }
            else
            {
                auto &img = resource_.emplace<VulkanImage>();
                img.init(d);
            }
        },
        desc
    );
}

uint PipelineResource::getBufferSlot() const
{
    return std::visit(
        [](auto &&r) -> uint
        {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, std::monostate>)
                return 0;
            else
                return r.description.bufferSlot;
        },
        resource_
    );
}

vk::DescriptorType PipelineResource::getDescriptorType() const
{
    return std::visit(
        [](auto &&r) -> vk::DescriptorType
        {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, std::monostate>)
                return vk::DescriptorType::eUniformBuffer;
            else
                return r.description.descriptorType;
        },
        resource_
    );
}

vk::ShaderStageFlags PipelineResource::getStageFlags() const
{
    return std::visit(
        [](auto &&r) -> vk::ShaderStageFlags
        {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, std::monostate>)
                return {};
            else
                return r.description.stageFlags;
        },
        resource_
    );
}

VulkanBuffer *PipelineResource::asBuffer()
{
    return std::get_if<VulkanBuffer>(&resource_);
}

VulkanImage *PipelineResource::asImage()
{
    return std::get_if<VulkanImage>(&resource_);
}

void PipelineResource::update(vk::DeviceSize bufferSize, void *data)
{
    std::visit(
        [&](auto &resource)
        {
            using T = std::decay_t<decltype(resource)>;

            if constexpr (!std::is_same_v<T, std::monostate>)
            {
                resource.update(bufferSize, data);
            }
        },
        resource_
    );
}
