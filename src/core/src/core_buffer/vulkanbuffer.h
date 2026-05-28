#pragma once
#include "core_buffer/ibuffer.h"
#include "core_render/linux/vulkanrenderer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct BufferAllocation
{
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDeviceSize size{0};
};

class VulkanBuffer : public IBuffer
{
    void init() override;
    void set() override;

    protected:
    template <typename T> void createBuffer(T);
    BufferAllocation allocateBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties
    );
    void *mapMemory(BufferAllocation &alloc, VkDeviceSize size);
    void unmapMemory(BufferAllocation &alloc);
    void freeBuffer(BufferAllocation &alloc);
    void createDescriptorSet();

    VulkanRenderer *renderer_;
    BufferAllocation uniformAlloc_;
};

class VulkanVertexBuffer : public VulkanBuffer
{
    BufferAllocation *
    allocateVertexBufferForGeometry(Geometry *geometry, uint32_t vertexCount);

    protected:
    std::unordered_map<Geometry *, BufferAllocation> vertexAllocs_;
};
