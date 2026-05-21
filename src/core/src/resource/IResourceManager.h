#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

struct BufferAllocation
{
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDeviceSize size{0};
};

class IResourceManager
{
    public:
    virtual ~IResourceManager() = default;

    virtual bool
    initialise(VkDevice device, VkPhysicalDevice physicalDevice) = 0;

    virtual BufferAllocation allocateBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ) = 0;

    virtual void *mapMemory(BufferAllocation &alloc, VkDeviceSize size) = 0;

    virtual void unmapMemory(BufferAllocation &alloc) = 0;

    virtual void freeBuffer(BufferAllocation &alloc) = 0;

    virtual void cleanup() = 0;
};
