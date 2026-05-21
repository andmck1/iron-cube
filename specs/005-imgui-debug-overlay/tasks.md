# Tasks: ImGui Debug Overlay

**Feature**: ImGui Debug Overlay
**Branch**: `005-imgui-debug-overlay`
**Spec**: [spec.md](./spec.md) | [plan.md](./plan.md)

---

## Phase 1: Setup

**Purpose**: Build system configuration for ImGui integration.

- [ ] T001 [P] Update `src/core/CMakeLists.txt`: add `imgui/src/imgui.cpp`, `imgui/src/imgui_draw.cpp`, `imgui/src/imgui_tables.cpp`, `imgui/src/imgui_widgets.cpp`, `imgui/backends/imgui_impl_vulkan.cpp`, `imgui/backends/imgui_impl_glfw.cpp` to `iron-core` library sources
- [ ] T002 [P] Update `src/core/CMakeLists.txt`: add `${imgui_SOURCE_DIR}` and `${imgui_SOURCE_DIR}/backends` to `iron-core` include directories

**Checkpoint**: Project compiles with ImGui sources linked into `iron-core`. ImGui headers are accessible.

---

## Phase 2: Foundational

**Purpose**: Core infrastructure — layer interface, scene transform data, renderer ImGui hooks, debug layer skeleton.

- [ ] T003 [P] Create `src/core/src/debug/ILayer.h`: define `ILayer` abstract interface with virtual `initialize()`, `render()`, and `shutdown()` methods
- [ ] T004 [P] Extend `src/core/src/scene/SceneData.h`: add `TriangleTransform` struct with `glm::vec2 position`, `float rotation` (degrees), `float scale`. Add default constructor initializing to `{glm::vec2(0,0), 0.0f, 1.0f}`
- [ ] T005 [P] Extend `src/core/src/scene/IScene.h`: add `getTriangleTransform() const`, `setTriangleTransform(const TriangleTransform&)`, and `hasTriangleTransformChanged() const` pure virtual methods
- [ ] T006 Extend `src/core/src/rendering/IRenderer.h`: add `initImGui()`, `newFrame()`, `renderImGui()`, and `shutdownImGui()` pure virtual methods for ImGui lifecycle hooks
- [ ] T007 Extend `src/core/src/rendering/linux/VulkanRenderer.h`: declare `initImGui()`, `newFrame()`, `renderImGui()`, and `shutdownImGui()` methods; add private ImGui member variables (`ImGuiContext*`, Vulkan descriptor pool, pipeline, font texture, etc.)
- [ ] T008 Extend `src/core/src/rendering/linux/VulkanRenderer.cpp`: implement `initImGui()` — create ImGui context, initialize Vulkan backend with device/queue, create font texture. Call from `init()` after swap chain setup.
- [ ] T009 Extend `src/core/src/rendering/linux/VulkanRenderer.cpp`: implement `newFrame()` — call backend new frame setup and `ImGui::NewFrame()`. Called at start of each render cycle before scene rendering.
- [ ] T010 Extend `src/core/src/rendering/linux/VulkanRenderer.cpp`: implement `renderImGui()` — record ImGui draw data into current frame's command buffer. Called after layer `render()` returns and before `swapBuffers()`.
- [ ] T011 Extend `src/core/src/rendering/linux/VulkanRenderer.cpp`: implement `shutdownImGui()` — clean up ImGui Vulkan resources (descriptor pool, pipeline, font texture). Called before `shutdown()` during teardown.
- [ ] T012 Create `src/core/src/debug/DebugLayer.h`: class implementing `ILayer`, member for FPS value, header guards and includes
- [ ] T013 Create `src/core/src/debug/DebugLayer.cpp`: stub implementations of `initialize()`, `render()`, and `shutdown()`
- [ ] T014 Update `src/core/CMakeLists.txt`: add `src/debug/DebugLayer.cpp` to `iron-core` library sources

**Checkpoint**: Foundation ready — layer interface defined, renderer supports ImGui frame lifecycle, debug layer skeleton compiles.

---

## Phase 3: User Story 1 — FPS Counter (Priority: P1) 🎯 MVP

**Goal**: Developer can see real-time FPS counter displayed on screen from application launch.

**Independent Test**: Launch application and verify an FPS readout is visible on screen, updating each frame.

### Implementation for User Story 1

- [ ] T015 [US1] Extend `src/app/src/Application.h`: add `std::array<float, 62>` frame time history ring buffer, write index, and `getFps()` method returning current rolling average FPS
- [ ] T016 [US1] Extend `src/app/src/Application.cpp` in `run()`: add frame timing with `std::chrono::high_resolution_clock` in the render loop, record frame times, compute rolling average over 62 frames
- [ ] T017 [US1] Extend `src/core/src/debug/DebugLayer.h` and `src/core/src/debug/DebugLayer.cpp`: add `setFps(float)` method and member variable; in `render()`, call `ImGui::Begin()`, `ImGui::Text("FPS: %.1f")`, `ImGui::End()` to display FPS counter

**Checkpoint**: FPS counter visible on screen, updating each frame, accurate within ±1 fps.

---

## Phase 4: User Story 2 — Triangle Transform Controls (Priority: P1) 🎯 MVP

**Goal**: Developer can modify triangle position, rotation, and scale via on-screen ImGui controls, with visual changes reflected within 1 frame.

**Independent Test**: Launch application, adjust transform sliders, observe triangle move/rotate/scale in real time, press reset button to restore defaults.

### Implementation for User Story 2

- [ ] T020 [P] Extend `src/core/src/scene/SceneData.h`: add `TriangleTransform transform` and `bool transform_dirty` fields to the scene data struct. Extend `src/core/src/scene/Scene.h`: declare `getTriangleTransform()`, `setTriangleTransform()`, `hasTriangleTransformChanged()`, and `markTransformClean()` methods that operate on the SceneData struct
- [ ] T021 [P] Extend `src/core/src/scene/Scene.cpp`: implement transform methods operating on SceneData — `getTriangleTransform()` reads from SceneData, `setTriangleTransform()` writes to SceneData and sets `transform_dirty`, `hasTriangleTransformChanged()` returns `transform_dirty`, `markTransformClean()` clears `transform_dirty`
- [ ] T019 [P] Extend `src/core/src/rendering/linux/VulkanRenderer.cpp` in `render()`: read current TriangleTransform from `IScene::getTriangleTransform()`, compute final transformation matrix, write to shader uniform buffer each frame. Call `IScene::markTransformClean()` after reading.
- [ ] T022 [US2] Extend `src/core/src/debug/DebugLayer.cpp` in `render()`: add ImGui slider controls for position X/Y (range -5.0 to 5.0), rotation (0° to 360°), and scale (0.1 to 10.0). Read current transform from `IScene` service via service locator, write modified values back
- [ ] T022b [US2] Extend `src/core/src/debug/DebugLayer.cpp` in `render()`: clamp slider values before writing to Scene — position to [-5.0, 5.0], scale to [0.1, 10.0], rotation unbounded
- [ ] T023 [US2] Extend `src/core/src/debug/DebugLayer.cpp` in `render()`: add ImGui reset button that restores triangle transform to defaults (centered position, 0° rotation, unit scale)

**Checkpoint**: Transform controls visible and interactive. Triangle responds to slider changes within 1 frame. Reset button restores defaults.

---

## Phase 5: User Story 3 — Debug Layer Integration (Priority: P2)

**Goal**: Debug overlay operates as a separate architectural layer via the layer stack pattern, cleanly separated from rendering and scene layers.

**Independent Test**: Verify Application manages a layer stack, iterates through layers in render loop, and debug layer interfaces only through IScene (not IRenderer internals).

### Implementation for User Story 3

- [ ] T024 [US3] Extend `src/core/src/scene/Scene.h` and `src/core/src/scene/Scene.cpp`: have `Scene` implement `ILayer` interface. Map `ILayer::initialize()` to existing `initialize()`, `ILayer::render()` as no-op (rendering is handled by VulkanRenderer), `ILayer::shutdown()` to existing `shutdown()`
- [ ] T025 [US3] Extend `src/app/src/Application.h`: add `std::vector<std::unique_ptr<ILayer>>` layer stack member, replace direct `Scene` member with `std::unique_ptr<Scene>`
- [ ] T026 [US3] Refactor `src/app/src/Application.cpp` in `run()`: create Scene and DebugLayer instances, push into layer stack, call `initialize()` on each layer during startup. In render loop: push FPS via `debugLayer->setFps(app.getFps())`, then call `renderer->newFrame()`, then call `render()` on each layer in order, then call `renderer->renderImGui()`, then `swapBuffers()`. On teardown call `shutdown()` on each layer in reverse order.
- [ ] T027 [US3] Refactor `src/app/src/Application.cpp` in `run()`: update service locator registration to use the dynamically allocated Scene instance instead of stack member

**Checkpoint**: Application uses layer stack pattern. Scene and DebugLayer are managed uniformly. Debug layer reads/writes scene data through IScene interface only.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Final validation and cleanup.

- [ ] T028 Validate build: clean build from scratch, verify no compiler warnings
- [ ] T029 Validate runtime: launch application, confirm FPS counter and transform controls both functional, verify debug overlay adds <1 ms frame overhead
- [ ] T030 Update `specs/005-imgui-debug-overlay/quickstart.md` with build/run instructions and debug overlay usage guide

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — blocks all user stories
  - T003, T004, T005 are independent of each other (different files)
  - T006–T011 are sequential (IRenderer contract, then VulkanRenderer implementation, each depends on prior)
  - T012–T014 depend on T003 (ILayer interface)
- **User Story 1 (Phase 3)**: Depends on Foundational phase completion
  - T015–T017 are sequential (Application timing → DebugLayer FPS display)
- **User Story 2 (Phase 4)**: Depends on Foundational phase completion
  - T020, T021, T019 can run in parallel (SceneData, Scene.cpp, VulkanRenderer)
  - T022 depends on T020 and T021 (DebugLayer reads/writes scene transform)
  - T022b depends on T022 (adds clamping to existing controls)
  - T023 depends on T022 (adds reset to existing controls)
- **User Story 3 (Phase 5)**: Depends on all prior user stories complete
  - T024 before T025 (Scene must implement ILayer before stack refactoring)
  - T025 before T026 (layer stack must exist before populating it)
  - T027 depends on T025 (service registration uses new Scene member)
- **Polish (Phase 6)**: Depends on all user stories complete

### Parallel Opportunities

- T001 and T002 (Setup): different CMake directives, same file — sequential
- T003, T004, T005 (Foundational): different files, fully parallel
- T020, T021 (US2): Scene.h and Scene.cpp — parallel
- T028, T029 (Polish): build validation and runtime validation — sequential (build first)

### Within Each User Story

- Core infrastructure before UI features
- Scene data model before DebugLayer consumption
- Story complete before moving to next priority

---

## Implementation Strategy

### MVP First (User Stories 1 + 2)

1. Complete Phase 1: Setup (ImGui build integration)
2. Complete Phase 2: Foundational (layer interface, renderer hooks, debug layer skeleton)
3. Complete Phase 3: FPS Counter → validate independently
4. Complete Phase 4: Transform Controls → validate independently
5. **MVP complete**: Debug overlay with FPS + transform controls working

### Full Delivery

6. Complete Phase 5: Layer stack refactoring → validate architectural separation
7. Complete Phase 6: Polish and validation

---

## Notes

- [P] tasks = different files, no cross-dependencies
- [US1]/[US2]/[US3] labels map tasks to specific user stories for traceability
- TDD suspended for this feature per user direction
- Commit after each task or logical group
- Stop at each checkpoint to validate independently
- VulkanRenderer modifications (T006–T011) must be sequential — each builds on the prior change in the same file
