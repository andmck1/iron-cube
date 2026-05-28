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

void VulkanBuffer::createDescriptorSet()
{
    spdlog::debug("Creating descriptor set");
    VulkanRenderer *renderer_ = dynamic_cast<VulkanRenderer *>(
        ServiceLocator::instance().get<IRenderer>()
    );
    const VkDevice device_ = renderer_->getDevice();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    poolInfo.flags = 0;

    vkCheck(
        vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_),
        "Failed to create descriptor pool"
    );

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout_;

    vkCheck(
        vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet_),
        "Failed to allocate descriptor sets"
    );

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformAlloc_.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UBO);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet_;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
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
    VkDevice device = renderer_->getDevice();

    void *data;
    VkResult result = vkMapMemory(device, alloc.memory, 0, size, 0, &data);
    if (result != VK_SUCCESS)
    {
        spdlog::error("Failed to map memory");
        return nullptr;
    }
    return data;
}

void VulkanBuffer::unmapMemory(BufferAllocation &alloc)
{
    VkDevice device = renderer_->getDevice();
    vkUnmapMemory(device, alloc.memory);
}

void VulkanBuffer::freeBuffer(BufferAllocation &alloc)
{
    VkDevice device = renderer_->getDevice();
    if (alloc.buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, alloc.buffer, nullptr);
        alloc.buffer = VK_NULL_HANDLE;
    }
    if (alloc.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, alloc.memory, nullptr);
        alloc.memory = VK_NULL_HANDLE;
    }
}

BufferAllocation *VulkanBuffer::allocateVertexBufferForGeometry(
    Geometry *geometry, uint32_t vertexCount
)
{
    VkDevice device = renderer_->getDevice();
    auto it = vertexAllocs_.find(geometry);
    if (it != vertexAllocs_.end())
    {
        VkDeviceSize currentSize = it->second.size;
        VkDeviceSize neededSize = sizeof(Vertex) * vertexCount;
        if (currentSize >= neededSize)
        {
            return &it->second;
        }

        VkResult waitResult = vkDeviceWaitIdle(device);
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
