# IRenderer Contract Delta

## Changes from Feature 004

The following methods are **added** to `IRenderer`:

### `initImGui()`

```cpp
virtual void initImGui() = 0;
```

Initializes ImGui context and Vulkan backend. Called after `init()` completes swap chain setup.

### `newFrame()`

```cpp
virtual void newFrame() = 0;
```

Starts a new ImGui frame. Called at the beginning of each render frame, before scene rendering.

### `renderImGui()`

```cpp
virtual void renderImGui() = 0;
```

Executes ImGui draw commands. Called after `ImGui::Render()`, before `swapBuffers()`.

### `shutdownImGui()`

```cpp
virtual void shutdownImGui() = 0;
```

Cleans up ImGui Vulkan resources (descriptor pools, pipelines, font texture). Called before `shutdown()`.

## Existing Methods

All existing methods from Feature 004 remain unchanged:

| Method | Status |
|--------|--------|
| `init(GLFWwindow*)` | Unchanged (Vulkan init) |
| `render()` | Unchanged (scene rendering) |
| `swapBuffers()` | Unchanged (present) |
| `shutdown()` | Unchanged (Vulkan cleanup) |

## VulkanRenderer Implementation Notes

- ImGui Vulkan resources are created after swap chain initialization (need `VkDevice`, descriptor pool limits, etc.)
- `newFrame()` sets up GLFW/Vulkan backend new frame callbacks
- `renderImGui()` records ImGui draw commands into the current frame's command buffer
- ImGui font texture is uploaded during `initImGui()` and cleaned up in `shutdownImGui()`
