#include "resource/ResourceManager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

bool ResourceManager::initialise(
    VkDevice device, VkPhysicalDevice physicalDevice
)
{
    device_ = device;
    physicalDevice_ = physicalDevice;
    spdlog::info("ResourceManager initialised");
    return true;
}

BufferAllocation ResourceManager::allocateBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryProps
)
{
    BufferAllocation alloc{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result =
        vkCreateBuffer(device_, &bufferInfo, nullptr, &alloc.buffer);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to create buffer: {}", static_cast<int>(result));
        return alloc;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, alloc.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, memoryProps);

    result = vkAllocateMemory(device_, &allocInfo, nullptr, &alloc.memory);
    if (result != VK_SUCCESS)
    {
        spdlog::error(
            "Failed to allocate buffer memory: {}", static_cast<int>(result)
        );
        vkDestroyBuffer(device_, alloc.buffer, nullptr);
        return alloc;
    }

    result = vkBindBufferMemory(device_, alloc.buffer, alloc.memory, 0);
    if (result != VK_SUCCESS)
    {
        spdlog::error(
            "Failed to bind buffer memory: {}", static_cast<int>(result)
        );
        vkFreeMemory(device_, alloc.memory, nullptr);
        vkDestroyBuffer(device_, alloc.buffer, nullptr);
        return alloc;
    }

    alloc.size = size;
    allocations_.push_back(alloc);
    return alloc;
}

void *ResourceManager::mapMemory(BufferAllocation &alloc, VkDeviceSize size)
{
    void *data;
    VkResult result = vkMapMemory(device_, alloc.memory, 0, size, 0, &data);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to map memory: {}", static_cast<int>(result));
        return nullptr;
    }
    return data;
}

void ResourceManager::unmapMemory(BufferAllocation &alloc)
{
    vkUnmapMemory(device_, alloc.memory);
}

void ResourceManager::freeBuffer(BufferAllocation &alloc)
{
    if (alloc.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device_, alloc.buffer, nullptr);
        alloc.buffer = VK_NULL_HANDLE;
    }
    if (alloc.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device_, alloc.memory, nullptr);
        alloc.memory = VK_NULL_HANDLE;
    }
    alloc.size = 0;
}

void ResourceManager::cleanup()
{
    for (BufferAllocation &alloc : allocations_)
    {
        if (alloc.buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device_, alloc.buffer, nullptr);
        }
        if (alloc.memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device_, alloc.memory, nullptr);
        }
    }
    allocations_.clear();
    device_ = VK_NULL_HANDLE;
    physicalDevice_ = VK_NULL_HANDLE;
    spdlog::info("ResourceManager cleaned up");
}

uint32_t ResourceManager::findMemoryType(
    uint32_t typeFilter, VkMemoryPropertyFlags properties
)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        bool typeBitMatch = typeFilter & (1 << i);
        bool propMatch = (memProperties.memoryTypes[i].propertyFlags &
                          properties) == properties;

        if (typeBitMatch && propMatch)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}
