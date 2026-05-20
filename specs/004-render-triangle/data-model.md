# Data Model: Render Triangle

## Entities

### Vertex

Represents a single vertex with position and color attributes.

| Field | Type | Description |
|-------|------|-------------|
| pos | glm::vec2 | Normalized device coordinates (-1 to +1) |
| color | glm::vec3 | RGB color values (0.0 to 1.0) |

**Validation**: pos components must be in [-1, +1]. Color components must be in [0.0, 1.0].

**Triangle vertices** (pointing up, centered):
- Vertex 0: pos = (-0.5, -0.5), color = (1.0, 0.0, 0.0) — bottom-left, bright red
- Vertex 1: pos = (0.5, -0.5), color = (1.0, 0.0, 0.0) — bottom-right, bright red
- Vertex 2: pos = (0.0, 0.5), color = (1.0, 0.0, 0.0) — top-center, bright red

### SceneData

Data-oriented container for renderable geometry.

| Field | Type | Description |
|-------|------|-------------|
| vertices | std::vector<Vertex> | Contiguous vertex array |
| vertex_count | uint32_t | Number of vertices (cached to avoid `.size()` call in render hot path) |
| is_dirty | bool | True when data has changed since last render (default: true) |

**Validation**: vertex_count must match vertices.size(). vertex_count must be > 0.

### VulkanDevice

Represents the Vulkan logical device and associated handles.

| Field | Type | Description |
|-------|------|-------------|
| instance | VkInstance | Vulkan instance handle |
| physical_device | VkPhysicalDevice | Selected GPU |
| device | VkDevice | Logical device handle |
| surface | VkSurfaceKHR | Window surface |
| graphics_queue | VkQueue | Graphics submission queue |
| queue_family_index | uint32_t | Graphics queue family index |

### SwapChainState

Tracks the current swap chain configuration.

| Field | Type | Description |
|-------|------|-------------|
| swap_chain | VkSwapchainKHR | Swap chain handle |
| images | std::vector<VkImage> | Swap chain images |
| image_views | std::vector<VkImageView> | Image views for each image |
| framebuffers | std::vector<VkFramebuffer> | Framebuffers for each image |
| format | VkFormat | Color format (B8G8R8A8_SRGB) |
| extent | VkExtent2D | Swap chain image dimensions |
| image_count | uint32_t | Number of images in chain |

### FrameState

Single-frame-in-flight synchronization state.

| Field | Type | Description |
|-------|------|-------------|
| command_buffer | VkCommandBuffer | Recorded command buffer for frame |
| command_pool | VkCommandPool | Command buffer pool |
| image_available_semaphore | VkSemaphore | Signals image ready for rendering |
| render_complete_semaphore | VkSemaphore | Signals rendering complete |
| fence | VkFence | Ensures frame completion before reuse |

### ResourceAllocation

Tracks a single GPU resource allocation.

| Field | Type | Description |
|-------|------|-------------|
| buffer | VkBuffer | GPU buffer handle |
| memory | VkDeviceMemory | Allocated device memory |
| size | VkDeviceSize | Allocation size in bytes |
| usage | VkBufferUsageFlags | Buffer usage flags |

## Relationships

```
SceneData
    │
    ├── contains → Vertex[] (3 vertices for triangle)
    │
    └── consumed by → RendererVulkan
                        │
                        ├── owns → VulkanDevice
                        ├── owns → SwapChainState
                         ├── owns → FrameState
                        └── uses → IResourceManager
                                        │
                                        └── tracks → ResourceAllocation[]
```

## State Transitions

### Vulkan Pipeline Lifecycle

```
Uninitialized
    ↓ init()
Instance Created
    ↓
Device Selected
    ↓
Device Created
    ↓
Surface Created
    ↓
Swap Chain Created
    ↓
Pipeline Ready
    ↓
[Frame Loop: record → submit → present → wait]
    ↓
Window Resize
    ↓
Swap Chain Recreated (Pipeline Ready restored)
    ↓
[Frame Loop continues]
    ↓ shutdown()
Resources Freed
    ↓
Uninitialized
```

### Scene Data Lifecycle

```
Empty
    ↓ initialize()
Triangle Data Generated (3 vertices)
    ↓
[Stable — data read by renderer each frame]
    ↓ shutdown()
Cleared
```

### Resource Allocation Lifecycle

```
Unallocated
    ↓ allocateBuffer()
Allocated (tracked in ResourceAllocation vector)
    ↓
[Used by renderer for vertex data / other buffers]
    ↓ cleanup()
Freed (removed from tracking vector)
```
