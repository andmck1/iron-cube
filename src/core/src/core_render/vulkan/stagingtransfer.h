#pragma once
#include "core_render/vulkan/renderer.h"
#include <vulkan/vulkan_raii.hpp>

class StagingTransfer
{
    public:
    void init(VulkanRenderer *renderer, vk::DeviceSize size);
    bool isInitialized() const;
    void upload(void *data, vk::DeviceSize size);
    void download(void *outData, vk::DeviceSize size);
    vk::raii::Buffer &buffer();
    vk::DeviceSize size() const;

    private:
    vk::raii::Buffer buffer_ = nullptr;
    vk::raii::DeviceMemory memory_ = nullptr;
    VulkanRenderer *renderer_ = nullptr;
    vk::DeviceSize size_ = 0;
};
