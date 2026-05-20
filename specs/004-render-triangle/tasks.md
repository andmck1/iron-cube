---
name: "Feature 004 - Render Triangle"
mode: "implement"
---

# Tasks: Render Triangle

## Phase 1: Setup

### T01 [P] Add Slang dependency to root CMakeLists.txt

Add `FetchContent_Declare` for Slang v2025.5 to root `CMakeLists.txt` alongside existing dependencies. Slang provides the `slangc` compiler needed for build-time shader compilation to SPIR-V.

**Files**: `CMakeLists.txt`

---

### T02 [P] Add Catch2 integration test target to tests/CMakeLists.txt

Add `add_subdirectory(integration)` to `tests/CMakeLists.txt` and create `tests/integration/CMakeLists.txt` with a `test_render_pipeline` executable linked against `Catch2::Catch2WithMain` and `iron-core`.

**Files**: `tests/CMakeLists.txt`, `tests/integration/CMakeLists.txt`

---

### T03 [P] Add Doxygen config for new source directories

Update `docs/Doxyfile` INPUT to include `src/core/src/scene/` and `src/core/src/resource/` so Doxygen documents the new scene and resource management layers.

**Files**: `docs/Doxyfile`

---

### T04 Add Slang shader compilation rules to src/core/CMakeLists.txt

Add CMake `add_custom_command` entries that invoke `slangc` to compile `src/core/src/shaders/triangle.vert.slang` and `triangle.frag.slang` to SPIR-V (`.spv`) in the build directory during the build. Mark the SPIR-V outputs as generated files. Use `DEPENDS` on the Slang library target to ensure `slangc` is available.

Command pattern:
```
slangc <shader>.slang -profile vulkan1.3 -target spirv -entry main -stage vertex -o <shader>.spv
```

**Files**: `src/core/CMakeLists.txt`

---

## Phase 2: Foundational

### T05 [P] Write unit tests for scene data structures

Create test file verifying: `Vertex` struct has correct memory layout (`glm::vec2 pos` + `glm::vec3 color`), `SceneData` default construction yields empty vertex vector with zero count, vertex insertion updates count, validation rejects vertex count mismatches, `IScene` interface contract (virtual destructor, `isDirty()` returns true after `initialize()`, `markClean()` resets dirty flag), and polymorphic deletion safety.

**Files**: `tests/unit/test_scene_data.cpp`

---

### T06 [P] Write unit tests for resource manager

Create test file verifying: `IResourceManager` interface contract (virtual destructor, method signatures), allocation tracking behavior, `cleanup()` frees all tracked resources, and error handling for null device initialization. Use mock or stub Vulkan handles where GPU is unavailable.

**Files**: `tests/unit/test_resource_manager.cpp`

---

### T07 Write unit tests for VulkanHelpers

Create test file verifying: `vkCheck()` throws `std::runtime_error` on non-success `VkResult` codes (`VK_ERROR_OUT_OF_HOST_MEMORY`, `VK_ERROR_DEVICE_LOST`, etc.), `vkCheck()` returns silently on `VK_SUCCESS` and `VK_SUBOPTIMAL`. Test message propagation in thrown exceptions. Note: low test ROI as `vkCheck()` is a thin wrapper — primary validation comes from integration coverage (T13, T27).

**Files**: `tests/unit/test_vulkan_helpers.cpp`

---

### T08 Implement Vertex struct and SceneData in SceneData.h

Define `struct Vertex { glm::vec2 pos; glm::vec3 color; }` and `struct SceneData { std::vector<Vertex> vertices; uint32_t vertex_count; }` with validation methods. Data stored contiguously in `std::vector` for cache-friendly access. No scene object hierarchy — pure data structures.

**Constitution**: Principle IV (Data-Oriented Design) — data in contiguous storage, not encapsulated in classes.

**Files**: `src/core/src/scene/SceneData.h`

---

### T09 [P] Implement IScene interface in IScene.h

Define pure virtual interface with `virtual ~IScene() = default`, `initialize()`, `getVertices()`, `getVertexCount()`, `shutdown()`, `isDirty()`, `markClean()`. Matches contract in `contracts/scene-contract.md`. Include Doxygen documentation.

**Constitution**: Principle V (Service-Locator Pattern) — interface-based service contract.

**Files**: `src/core/src/scene/IScene.h`

---

### T09b [P] Implement IResourceManager interface in IResourceManager.h

Define pure virtual interface with `initialize()`, `allocateBuffer()`, `mapMemory()`, `cleanup()`. Matches contract in `contracts/resource-manager-contract.md`. Include Doxygen documentation.

**Constitution**: Principle V (Service-Locator Pattern) — interface-based service contract.

**Files**: `src/core/src/resource/IResourceManager.h`

---

### T10 Implement Scene class with triangle generation

Create `class Scene : public IScene` in `Scene.h`/`Scene.cpp`. `initialize()` procedurally generates 3 vertices forming a centered, upward-pointing triangle with bright red color `(1.0, 0.0, 0.0)`, sets `is_dirty = true`. Vertices: `(-0.5, -0.5)`, `(0.5, -0.5)`, `(0.0, 0.5)`. Data stored in `SceneData` struct. `getVertices()` returns const reference. `isDirty()` returns `is_dirty` flag. `markClean()` sets `is_dirty = false`. `shutdown()` clears all data.

**Files**: `src/core/src/scene/Scene.h`, `src/core/src/scene/Scene.cpp`

---

### T10b Implement ResourceManager class

Create `class ResourceManager : public IResourceManager` in `ResourceManager.h`/`ResourceManager.cpp`. Tracks `VkDevice` and `VkPhysicalDevice` handles. `allocateBuffer()` creates `VkBuffer` with `VkDeviceMemory`, tracks allocation in internal vector. `mapMemory()` maps device memory for CPU write access. `cleanup()` iterates tracked allocations and frees all buffers and memory. Throws `std::runtime_error` on fatal errors.

**Constitution**: Principle III (Layered Architecture) — resource management layer below rendering layer.

**Files**: `src/core/src/resource/ResourceManager.h`, `src/core/src/resource/ResourceManager.cpp`

---

### T11 Implement VulkanHelpers.h

Header-only module providing: `vkCheck(VkResult result, const char* msg)` inline function that throws `std::runtime_error(msg)` when `result != VK_SUCCESS`; debug utils messenger setup function `setupDebugMessenger(VkInstance, VkDebugUtilsMessengerCreateInfoEXT*)`; and `#ifdef DEBUG` conditional compilation for validation layer enablement. All Vulkan error paths centralized here.

**Files**: `src/core/src/rendering/linux/VulkanHelpers.h`

---

## Phase 3: User Story 1 — Rendering Pipeline Execution

**Goal**: The rendering pipeline initializes successfully, executes continuously without crashes, handles errors gracefully, and cleans up all resources on exit.

### T12 [P] Write unit tests for RendererVulkan initialization

Create test file verifying: `RendererVulkan` implements `IRenderer` interface (compile-time check via `std::is_base_of`), Vulkan instance creation succeeds when GPU is available, `init()` returns false with logged error when no Vulkan GPU found, and initialization failure triggers clean shutdown without resource leaks. Use `TEST_CASE_SKIP` with `#ifdef HEADLESS_CI` to gracefully skip GPU-dependent tests in headless CI environments.

**Files**: `tests/unit/test_renderer_vulkan.cpp`

---

### T13 Write integration test skeleton for full render pipeline

Create integration test file with `TEST_CASE_SKIP` stubs that will verify: complete pipeline lifecycle (init → render loop for N frames → swap → shutdown), no Vulkan validation layer errors during rendering, renderer reads scene data from `IScene` service via locator, and resource manager properly tracks and cleans up GPU allocations. Test compiles but is skipped until T14-T21 are implemented. Full test body is filled in after T21 completes.

**Files**: `tests/integration/test_render_pipeline.cpp`

---

### [X] T14 Implement RendererVulkan class skeleton and init()

Create `class RendererVulkan : public IRenderer` in `RendererVulkan.h`/`RendererVulkan.cpp`. Implement `init(GLFWwindow*)` method: create `VkInstance` with GLFW surface extensions and validation layers (Debug), select `VkPhysicalDevice` (discrete GPU preferred), create `VkDevice` with graphics queue family and swapchain extension, create `VkSurfaceKHR` from GLFW window. Store handles in `VulkanDevice` struct. Use `vkCheck()` for all Vulkan calls. Return false on any failure. Update `IRenderer.h` Doxygen comments to document Vulkan-specific behavior: validation layers in Debug builds, exception semantics via `vkCheck()`, and `IScene` service dependency for `render()`.

**Constitution**: Principle III (Layered Architecture) — rendering layer implementation. Principle V (Service-Locator Pattern) — implements `IRenderer` interface.

**Files**: `src/core/src/rendering/linux/RendererVulkan.h`, `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T15 Implement swap chain and render pass setup

Extend `RendererVulkan::init()` to create swap chain with `B8G8R8A8_SRGB` format, create image views for each swap chain image, create single-color-attachment render pass (clear to black), and create framebuffers (one per swap chain image). Store state in `SwapChainState` struct.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T16 Implement graphics pipeline and shader loading

Extend `RendererVulkan::init()` to load pre-compiled SPIR-V shaders (`.spv` files from build directory), create vertex input state matching `Vertex` struct layout (`glm::vec2 pos` + `glm::vec3 color`), create graphics pipeline with vertex input, input assembly (triangle list), viewport/state, rasterizer, color blend (clear to black), and depth-stencil (disabled). Verify shader files exist; return false with logged error if missing.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T17 Implement vertex buffer creation via resource manager

During `init()`, query `IResourceManager` from service locator, call `allocateBuffer()` to create vertex buffer with `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`, map memory, copy triangle vertex data from `IScene` service, and unmap. Store buffer handle and device pointer for rendering.

**Constitution**: Principle IV (Data-Oriented Design) — scene data flows from scene layer to rendering layer via data structures, not object method calls.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T18 Implement command pool and command buffer creation

During `init()`, create `VkCommandPool` for graphics queue family and allocate one `VkCommandBuffer`. Store in `FrameState` struct alongside semaphore and fence placeholders for single-frame-in-flight synchronization.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T19 Implement render() — frame command recording

Implement `RendererVulkan::render()`: reset command buffer, begin render pass with clear color `(0.0, 0.0, 0.0, 1.0)`, bind graphics pipeline, update vertex buffer from `IScene` service if `scene->isDirty()` is true (then call `scene->markClean()`), bind vertex buffer, issue `vkCmdDraw(3)`, end render pass, end command buffer.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T20 Implement swapBuffers() — submit and present

Implement `RendererVulkan::swapBuffers()`: acquire next image from swap chain, wait on image-available semaphore, submit command buffer with semaphore dependencies (wait: image-available, signal: render-complete), present image to swap chain with render-complete semaphore, wait on fence. Create per-frame semaphores and fence if not yet created.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T21 Implement error handling and graceful shutdown

Implement `RendererVulkan::shutdown()`: call `vkDeviceWaitIdle()`, destroy command pool, destroy render pipeline, destroy framebuffers, destroy render pass, destroy swap chain, destroy device, destroy instance. Call `IResourceManager::cleanup()` via locator to free GPU allocations. All destruction in reverse initialization order. No resource leaks.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

## Phase 4: User Story 2 — Triangle Rendering

**Goal**: A bright red triangle is rendered on a black background, window resize is handled correctly, and rendering pauses when minimized.

### T22 Implement window resize handler

Add GLFW framebuffer resize callback that: calls `vkDeviceWaitIdle()`, destroys swap chain, framebuffers, and image views, recreates swap chain with new extent, recreates framebuffers and image views, restores `SwapChainState`. Renderer remains functional after resize with correct triangle rendering.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### T23 Implement minimized window detection

In `render()` and `swapBuffers()`, check `glfwGetWindowAttrib(window, GLFW_ICONIFIED)` at start of each method. Return immediately (no-op) when window is iconified. Vulkan swap chain operations are skipped, preventing errors on minimized windows.

**Files**: `src/core/src/rendering/linux/RendererVulkan.cpp`

---

### [X] T24 Write Slang vertex shader

Create `triangle.vert.slang` with `[stage("vertex")]` entry point `main()`. Input `pos` (vec2) via `[[location(0)]]` and `color` (vec3) via `[[location(1)]]`. Output `gl_Position` (vec4) and `color` (vec3) via `[[location(0)]]` and `[[location(1)]]` respectively. Vertex data comes from vertex buffer input attributes matching `Vertex` struct layout.

**Files**: `src/core/src/shaders/triangle.vert.slang`

---

### [X] T25 Write Slang fragment shader

Create `triangle.frag.slang` with `[stage("fragment")]` entry point `main()`. Interpolate `color` from vertex shader output using `[[location(1)]]` attribute. Return interpolated color as `float4` fragment color.

**Files**: `src/core/src/shaders/triangle.frag.slang`

---

### [X] T26 Update Application to use RendererVulkan and IScene

Replace `RendererGLFW` with `RendererVulkan` in `Application.h`/`Application.cpp`. Register `Scene` as `IScene` service and `ResourceManager` as `IResourceManager` service in service locator. Call `scene->initialize()` after renderer init. Call `renderer->render()` before `renderer->swapBuffers()` each frame. Add `try-catch` block in `run()` to catch `std::runtime_error`, log error via spdlog, call shutdown on all services, and return non-zero exit code. Remove `RendererGLFW` from `iron-core` source list in CMake. Remove `src/core/src/rendering/linux/RendererGLFW.h` and `RendererGLFW.cpp` files.

**Files**: `src/app/src/Application.h`, `src/app/src/Application.cpp`, `src/core/CMakeLists.txt`

---

### T27 Write memory leak validation test

Create test verifying: after full lifecycle (init → N frames → shutdown), no Vulkan objects remain alive. Use Vulkan validation layer `VK_DEBUG_REPORT_OBJECT_LOST_EXT` or equivalent to detect leaked handles. Verify `IResourceManager::cleanup()` frees all tracked allocations.

**Files**: `tests/integration/test_render_pipeline.cpp`

---

## Dependencies and Execution Order

```
Phase 1 (Setup):
  T01, T02, T03 [parallel] → T04

Phase 2 (Foundational):
  T05, T06, T07 [parallel]
  T08 → T09, T09b [parallel]
  T09 → T10
  T09b → T10b
  T11 [parallel with T09-T10b]

Phase 3 (US1 - Rendering Pipeline):
  T12 [parallel with T14]
  T14 → T15 → T16 → T17 → T18 → T19 → T20 → T21
  T21 → T13 (integration test skeleton written in Phase 3 start, filled in after T21)

Phase 4 (US2 - Triangle Rendering):
  T22, T23 [parallel]
  T24, T25 [parallel]
  T24, T25 → T04 (shader source files must exist before CMake compilation rules are added)
  T24, T25 → T26 (shaders must exist before Application integration)
  T21, T23 → T26 (renderer must be complete before Application integration)
  T26 → T27
```

## TDD Compliance Notes

- **Every implementation task is preceded by its test task**: T05→T08/T09, T06→T09b/T10b, T07→T11, T12→T14-T21
- **Tests must fail before implementation**: Each test file should compile and fail (or be skipped with `TEST_CASE_SKIP`) before the corresponding implementation exists
- **Integration test (T13)**: Skeleton test file created early in Phase 3 with `TEST_CASE_SKIP` stubs. Full test body is filled in after T21 completes, validating the complete pipeline end-to-end
- **Memory leak test (T27)**: Final validation gate after all implementation is complete. Not a TDD test-first item — validates absence of leaks post-implementation
- **Headless CI**: T12 and T13 use `TEST_CASE_SKIP` with `#ifdef HEADLESS_CI` to gracefully skip GPU-dependent tests

## Constitution Compliance

| Principle | Coverage |
|-----------|----------|
| I. TDD | Test tasks (T05-T07, T12) precede implementation; T13 skeleton test created early, filled after T21; T27 validates post-implementation |
| II. Documentation First | Contracts, data-model, research completed; Doxygen comments required on all new headers (T14 updates IRenderer.h) |
| III. Layered Architecture | Rendering layer (T14-T21), resource layer (T10b), scene layer (T10) — each depends only on layers below |
| IV. Data-Oriented Design | Scene data in contiguous `std::vector` (T08, T10), no scene object hierarchy, AoS layout for GPU compatibility |
| V. Service-Locator Pattern | `IScene`, `IResourceManager`, `IRenderer` interfaces (virtual destructors) consumed via `ServiceLocator` (T09, T09b, T10, T10b, T14, T26) |
| VI. Incremental Development | 4 phases, each independently testable; US1 validates pipeline, US2 validates visual output |
