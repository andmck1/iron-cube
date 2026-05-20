# Resource Manager Service Contract

## Interface: IResourceManager

**Location**: `src/core/src/resource/IResourceManager.h`

### Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| initialize | `bool initialize(VkDevice device, VkPhysicalDevice physicalDevice)` | Initialize with Vulkan device handles. Returns true on success. |
| allocateBuffer | `bool allocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer* outBuffer, VkDeviceMemory* outMemory)` | Allocate a GPU buffer with backing memory. Returns true on success. |
| mapMemory | `void* mapMemory(VkDeviceMemory memory, VkDeviceSize size)` | Map device memory for CPU access. Returns pointer or nullptr. |
| cleanup | `void cleanup()` | Free all tracked allocations and reset state. |

### Behavior

- All allocations tracked internally for batch cleanup
- `cleanup()` must be called before Application exits
- Memory properties selected automatically: HOST_VISIBLE | HOST_COHERENT for upload buffers, DEVICE_LOCAL for GPU-only buffers
- Allocation failure returns false (does not throw)

### Error Cases

| Condition | Behavior |
|-----------|----------|
| Device is null | `initialize()` returns false |
| Out of device memory | `allocateBuffer()` returns false |
| Memory map fails | `mapMemory()` returns nullptr |
