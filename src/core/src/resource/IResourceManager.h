/**
 * @file
 * @brief Resource Manager interface for GPU resource tracking.
 *
 * Provides buffer allocation, memory mapping, and cleanup
 * for Vulkan GPU resources.
 */

#pragma once

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

/**
 * @brief Buffer allocation handle.
 *
 * Stores buffer and memory handles along with
 * size information for tracked allocations.
 */
struct BufferAllocation
{
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDeviceSize size{0};
};

/**
 * @brief Resource Manager interface.
 *
 * Manages GPU buffer allocations and memory,
 * providing centralized resource tracking
 * for proper cleanup.
 *
 * @par Thread Safety
 * Not thread-safe. All calls should be made
 * from the main thread during initialisation
 * and shutdown.
 *
 * @par Usage
 * @code
 * resourceManager->initialise(device, physicalDevice);
 * auto alloc = resourceManager->allocateBuffer(size, usage);
 * void* ptr = resourceManager->mapMemory(alloc, size);
 * // ... use buffer ...
 * resourceManager->cleanup();
 * @endcode
 */
class IResourceManager
{
    public:
    virtual ~IResourceManager() = default;

    /**
     * @brief Initialise resource manager with device handles.
     * @param device Logical device handle.
     * @param physicalDevice Physical device handle.
     * @return true on success, false on failure.
     */
    virtual bool initialise(VkDevice device, VkPhysicalDevice physicalDevice) = 0;

    /**
     * @brief Allocate a GPU buffer with dedicated memory.
     * @param size Buffer size in bytes.
     * @param usage Buffer usage flags.
     * @param memoryProps Required memory property flags.
     * @return BufferAllocation handle on success, null handle on failure.
     */
    virtual BufferAllocation allocateBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ) = 0;

    /**
     * @brief Map buffer memory for CPU access.
     * @param alloc Buffer allocation to map.
     * @param size Size to map in bytes.
     * @return Pointer to mapped memory, or nullptr on failure.
     */
    virtual void* mapMemory(BufferAllocation& alloc, VkDeviceSize size) = 0;

    /**
     * @brief Unmap previously mapped buffer memory.
     * @param alloc Buffer allocation to unmap.
     */
    virtual void unmapMemory(BufferAllocation& alloc) = 0;

    /**
     * @brief Free a single buffer allocation.
     * @param alloc Buffer allocation to free.
     */
    virtual void freeBuffer(BufferAllocation& alloc) = 0;

    /**
     * @brief Clean up all tracked allocations.
     */
    virtual void cleanup() = 0;
};
