# Research: ImGui Debug Overlay

## Decision 1: ImGui Vulkan Backend Integration

**Decision**: Use ImGui's built-in Vulkan backend (`imgui_impl_vulkan.cpp/h`) with the existing `VulkanRenderer`.

**Rationale**: ImGui provides a tested Vulkan backend that handles descriptor pool management, pipeline caching, font texture upload, and draw command recording. Building a custom integration would duplicate this work.

**Alternatives considered**:
- Custom ImGui renderer: Rejected — unnecessary complexity, ImGui's Vulkan backend is mature and well-maintained
- SDL backend: Rejected — project uses GLFW, not SDL
- OpenGL backend: Rejected — project uses Vulkan exclusively

**Integration approach**:
- `VulkanRenderer::init()` creates ImGui context, initializes Vulkan backend after swap chain creation
- Per-frame: `ImGui::NewFrame()` before scene render, `ImGui::Render()` + custom draw data execution after scene render
- ImGui Vulkan resources (descriptor pools, pipelines, font texture) stored in `VulkanRenderer`
- `VulkanRenderer::shutdown()` cleans up ImGui resources before Vulkan device cleanup

## Decision 2: Layer Stack Pattern

**Decision**: Introduce `ILayer` interface with `initialize()`, `render()`, and `shutdown()` methods. Application maintains an ordered `std::vector<std::unique_ptr<ILayer>>`.

**Rationale**: The layer stack provides a clean extension point for future debug/developer tools without modifying the render loop. Each layer is self-contained and composable.

**Alternatives considered**:
- Direct ImGui calls in Application: Rejected — couples Application to ImGui, violates layer separation
- Callback system: Rejected — more complex than needed, layer stack is simpler and more extensible
- Observer pattern: Rejected — layers need ordered execution (scene before debug overlay), observers don't guarantee order

**Layer ordering**:
1. Scene layer — renders the triangle (and future scene content)
2. Debug layer — renders ImGui overlay on top

## Decision 3: Transform Data Model

**Decision**: Extend `SceneData.h` with a `TriangleTransform` struct containing `glm::vec2 position`, `float rotation` (degrees), and `float scale`. Scene stores and exposes this data via `IScene`.

**Rationale**: The transform data belongs in the scene layer as it describes how scene objects appear. The debug layer reads and writes this data through `IScene`, maintaining architectural boundaries.

**Alternatives considered**:
- Store transform in debug layer: Rejected — transform is scene state, not debug state
- Use full 4x4 matrix: Rejected — overkill for 2D screen-space transform; simple struct is clearer for debug UI

## Decision 4: FPS Calculation Method

**Decision**: Rolling average over the last 62 frames (~1 second at 60 fps), computed in the Application's main loop and passed to the debug layer each frame.

**Rationale**: A rolling average smooths minor frame-time jitter while remaining responsive to sustained changes. 62 frames provides ~1 second of history at 60 fps.

**Alternatives considered**:
- Instantaneous FPS (1/deltaTime): Rejected — too noisy, fluctuates wildly frame-to-frame
- Exponential moving average: Rejected — slightly more complex, rolling average is sufficient
- Debug layer computes its own FPS: Rejected — Application owns the frame timing, debug layer should consume the metric

## Decision 5: ImGui Build Integration

**Decision**: ImGui sources are already fetched via FetchContent. Add `imgui` and `imgui_backends` to the `iron-core` library link dependencies. Include `imgui_impl_vulkan.cpp` and `imgui_impl_glfw.cpp` in the core library build.

**Rationale**: ImGui is already available as a FetchContent dependency. The Vulkan+GLFW backend sources need to be compiled into `iron-core` alongside `VulkanRenderer`.

**Alternatives considered**:
- Link ImGui as separate library: Rejected — ImGui is header-heavy with minimal separate library benefit
- Bundle ImGui sources: Rejected — FetchContent already handles versioning and updates

## Decision 6: IScene Interface Extension

**Decision**: Add `getTriangleTransform()`, `setTriangleTransform()`, and `hasTriangleTransformChanged()` to `IScene`.

**Rationale**: The debug layer needs to read current transform values for UI display and write modified values back. The dirty-check pattern (`hasTriangleTransformChanged()`) mirrors the existing `isDirty()`/`markClean()` pattern for vertex data.

**Alternatives considered**:
- Generic transform API: Rejected — feature scope is triangle-specific; generic API is premature
- Direct data access: Rejected — violates encapsulation; interface methods maintain layer boundaries
