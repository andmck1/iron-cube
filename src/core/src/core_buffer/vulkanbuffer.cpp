#include "core_buffer/vulkanbuffer.h"
#include "core_render/linux/vulkanhelpers.h"
#include "core_render/linux/vulkanrenderer.h"
#include "core_service/servicelocator.h"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

void VulkanBuffer::init()
{
    renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
}

template <typename T> void VulkanBuffer::createBuffer(T)
{
    spdlog::debug("Creating uniform buffer");
    VkDeviceSize bufferSize = sizeof(T);

    uniformAlloc_ = allocateBuffer(
        bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    if (uniformAlloc_.buffer == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to allocate uniform buffer");
}

BufferAllocation VulkanBuffer::allocateBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties
)
{
    VkDevice device = renderer_->getDevice();
    BufferAllocation alloc{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result =
        vkCreateBuffer(device, &bufferInfo, nullptr, &alloc.buffer);
    vkCheck(result, "Failed to create buffer");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, alloc.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        renderer_->findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(device, &allocInfo, nullptr, &alloc.memory);
    vkCheck(result, "Failed to allocate buffer memory");

    vkBindBufferMemory(device, alloc.buffer, alloc.memory, 0);
    alloc.size = size;
    return alloc;
}

void *VulkanBuffer::mapMemory(BufferAllocation &alloc, VkDeviceSize size)
{
    void *data;
    VkResult result = vkMapMemory(device_, alloc.memory, 0, size, 0, &data);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to map memory");
        return nullptr;
    }
    return data;
}

void VulkanBuffer::unmapMemory(BufferAllocation &alloc)
{
    vkUnmapMemory(device_, alloc.memory);
}

void VulkanBuffer::freeBuffer(BufferAllocation &alloc)
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
}

BufferAllocation *VulkanBuffer::allocateVertexBufferForGeometry(
    Geometry *geometry, uint32_t vertexCount
)
{
    // Check if we already have a buffer for this geometry
    auto it = vertexAllocs_.find(geometry);
    if (it != vertexAllocs_.end())
    {
        // Reuse existing buffer if size matches
        VkDeviceSize currentSize = it->second.size;
        VkDeviceSize neededSize = sizeof(Vertex) * vertexCount;
        if (currentSize >= neededSize)
        {
            return &it->second;
        }

        // Need to resize - wait for GPU, free old, allocate new
        VkResult waitResult = vkDeviceWaitIdle(device_);
        if (waitResult != VK_SUCCESS && waitResult != VK_ERROR_DEVICE_LOST)
        {
            throw std::runtime_error(
                std::string("Device wait idle failed") +
                " (VkResult: " + std::to_string(waitResult) + ")"
            );
        }
        freeBuffer(it->second);
    }

    VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
    if (bufferSize > 10000)
        return nullptr;
    spdlog::info(
        "Allocating vertex buffer: {} vertices, {} bytes", vertexCount,
        bufferSize
    );

    BufferAllocation alloc = allocateBuffer(
        bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    if (alloc.buffer == VK_NULL_HANDLE)
        return nullptr;

    vertexAllocs_[geometry] = alloc;
    return &vertexAllocs_[geometry];
}
