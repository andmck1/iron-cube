/**
 * @file
 * @brief Resource Manager implementation for GPU resource tracking.
 */

#include "resource/IResourceManager.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

class ResourceManager : public IResourceManager
{
    public:
    bool initialise(VkDevice device, VkPhysicalDevice physicalDevice) override;
    BufferAllocation allocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                    VkMemoryPropertyFlags memoryProps) override;
    void* mapMemory(BufferAllocation& alloc, VkDeviceSize size) override;
    void unmapMemory(BufferAllocation& alloc) override;
    void freeBuffer(BufferAllocation& alloc) override;
    void cleanup() override;

    private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice device_{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    std::vector<BufferAllocation> allocations_;
};
