// AI help here

#pragma once
#include "core_render/vulkan/renderer.h"
#include "core_render/vulkan/stagingtransfer.h"
#include <vulkan/vulkan_raii.hpp>

struct ImageDescription
{
    uint bufferSlot;
    vk::DescriptorType descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
    vk::Extent3D extent{1, 1, 1};
    vk::Format format = vk::Format::eR8G8B8A8Srgb;
    vk::ImageUsageFlags usage =
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::MemoryPropertyFlags deviceMemory =
        vk::MemoryPropertyFlagBits::eDeviceLocal;
    vk::ShaderStageFlags stageFlags;
    void *data = nullptr;
    vk::DeviceSize dataSize = 0;
    bool createSampler = true;
    vk::Filter filter = vk::Filter::eLinear;
    vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
    uint32_t arrayLayers = 1;
    uint32_t mipLevels = 1;
    vk::ImageCreateFlags flags = {};
    vk::ImageViewType viewType = vk::ImageViewType::e2D;
    std::vector<vk::BufferImageCopy> copyRegions;

    ImageDescription &setBufferSlot(uint slot)
    {
        bufferSlot = slot;
        return *this;
    }
    ImageDescription &setDescriptorType(vk::DescriptorType type)
    {
        descriptorType = type;
        return *this;
    }
    ImageDescription &setExtent(vk::Extent3D e)
    {
        extent = e;
        return *this;
    }
    ImageDescription &setFormat(vk::Format f)
    {
        format = f;
        return *this;
    }
    ImageDescription &setUsage(vk::ImageUsageFlags u)
    {
        usage = u;
        return *this;
    }
    ImageDescription &setAspectMask(vk::ImageAspectFlags a)
    {
        aspectMask = a;
        return *this;
    }
    ImageDescription &setDeviceMemory(vk::MemoryPropertyFlags memory)
    {
        deviceMemory = memory;
        return *this;
    }
    ImageDescription &setStageFlags(vk::ShaderStageFlags flags)
    {
        stageFlags = flags;
        return *this;
    }
    ImageDescription &setData(void *d)
    {
        data = d;
        return *this;
    }
    ImageDescription &setDataSize(vk::DeviceSize s)
    {
        dataSize = s;
        return *this;
    }
    ImageDescription &setCreateSampler(bool b)
    {
        createSampler = b;
        return *this;
    }
    ImageDescription &setFilter(vk::Filter f)
    {
        filter = f;
        return *this;
    }
    ImageDescription &setAddressMode(vk::SamplerAddressMode m)
    {
        addressMode = m;
        return *this;
    }
    ImageDescription &setArrayLayers(uint32_t layers)
    {
        arrayLayers = layers;
        return *this;
    }
    ImageDescription &setMipLevels(uint32_t levels)
    {
        mipLevels = levels;
        return *this;
    }
    ImageDescription &setFlags(vk::ImageCreateFlags f)
    {
        flags = f;
        return *this;
    }
    ImageDescription &setViewType(vk::ImageViewType t)
    {
        viewType = t;
        return *this;
    }
    ImageDescription &setCopyRegions(std::vector<vk::BufferImageCopy> regions)
    {
        copyRegions = std::move(regions);
        return *this;
    }
};

struct FrameImage
{
    vk::raii::Image image = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    vk::raii::ImageView view = nullptr;
    StagingTransfer staging;
};

class VulkanImage
{
    public:
    void init(ImageDescription imageDescription);
    void update(vk::DeviceSize size, void *data);

    vk::raii::Image *getImage(uint32_t frameIndex);
    vk::raii::ImageView *getImageView(uint32_t frameIndex);
    vk::raii::Sampler *getSampler();
    FrameImage *getFrameImage(uint32_t frameIndex);

    ImageDescription description;

    protected:
    std::vector<FrameImage> frames_;
    vk::raii::Sampler sampler_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;
};
