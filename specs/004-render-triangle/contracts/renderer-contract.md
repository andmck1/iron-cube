# Renderer Service Contract

## Interface: IRenderer

**Location**: `src/core/src/rendering/IRenderer.h`

### Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| init | `bool init(GLFWwindow *window)` | Initialize Vulkan instance, device, swap chain, pipeline. Returns true on success. |
| render | `void render()` | Record and submit draw commands for current frame. Reads scene data from IScene service. |
| swapBuffers | `void swapBuffers()` | Present current frame via swap chain. |
| shutdown | `void shutdown()` | Destroy all Vulkan resources in reverse initialization order. |

### Behavior

- `init()` must be called before any other method
- `render()` + `swapBuffers()` called once per frame in main loop
- Window resize automatically handled via GLFW callback
- Minimized window: `render()` and `swapBuffers()` are no-ops
- Vulkan validation layers enabled in Debug builds
- All VkResult errors throw `std::runtime_error` caught by Application layer

### Error Cases

| Condition | Behavior |
|-----------|----------|
| No Vulkan-capable GPU | `init()` returns false, Application exits with code 1 |
| Shader SPIR-V files missing | `init()` returns false with logged error |
| Swap chain creation fails | `init()` returns false with logged error |
| Out of device memory | Exception thrown, caught by Application, clean shutdown |

## Helper Module: VulkanHelpers

**Location**: `src/core/src/rendering/linux/VulkanHelpers.h`

### Scope

- `vkCheck(VkResult, const char*)`: Inline function to check VkResult and throw `std::runtime_error` on failure
- Debug utils messenger callback: Logs validation layer warnings and errors via spdlog
- Debug builds enable Vulkan validation layers; release builds disable them
