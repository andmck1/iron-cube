# Task Specification: GFX Renderer Viewer Implementation Plan

**Feature**: GFX Renderer Viewer (Iron Cube)
**Target State**: Fully functional, modular renderer with real-time debug capabilities and plugin extensibility.
**Architecture Focus**: Plugin-based, Layered (Renderer $\to$ Core Manager $\to$ Contracts).

## Dependencies Summary
*   **Blocking Prerequisite**: Completion of Phase 1 tasks are mandatory before any story can begin.
*   **Critical Contracts**: `IRenderConfigurator`, `CentralCoreManager` API, and the RPC Bus definition MUST be stable first.

---
## Phase 1: Setup & Foundational Infrastructure (No Story Dependency)

This phase establishes the project structure and core services needed by every subsequent task.

- [ ] T001 Initialize build system with CMake/Ninja for modularity (`CMakeLists.txt` in root, libraries' `CMakeLists.txt`). Path: `CMakeLists.txt`
- [P] T002 Establish base project structure and module scaffolding (e.g., `core`, `renderer`, `plugin_api`). Path: `src/module/`
- [ ] T003 Implement basic logging utility using spdlog with structured logging capability. Path: `src/util/logger.cpp`
- [P] T004 Define and implement the core RPC Message Bus client/server stub using gRPC for inter-process communication. Path: `src/rpc/message_bus.h`
- [ ] T005 Implement the Central Core Manager singleton class responsible for initializing, registering, and managing global state (Debug Settings). Path: `src/core/central_manager.cpp`

## Phase 2: Foundational Services & Contracts Implementation (Blocking)

These services must be operational before any user story can advance.

- [P] T010 Implement the basic structure for the `IRenderConfigurator` interface and its boilerplate in C++/Python. Path: `src/contracts/interface_renderer_configurator.h`
- [ ] T011 Implement data serialization/deserialization layer (e.g., JSON schema validation) used by the Central Core Manager to manage settings. Path: `src/util/serializer.cpp`
- [P] T012 Implement a mockable, thread-safe implementation of the Debug Settings structure within the Central Core Manager. Path: `src/core/debug_settings.h`

## Phase 3: User Story Implementation (Priority Order)

### **[US1] Viewer Initialization and Basic Rendering (P1)**
*Goal*: Achieve a stable, functional display of basic geometry upon startup.

- [ ] T020 Core module initialization to create the Vulkan context/renderer object. Path: `src/renderer/vulkan_context.cpp`
- [P] T021 Implement core pipeline setup: loading shaders (Slang) and creating the primary render pass for a colored triangle. Path: `src/renderer/basic_pass.cpp`
- [ ] T022 Create the basic UI shell structure hosting the canvas view (using imGUI). Path: `src/ui/main_gui.cpp`

### **[US2] Real-Time Debug Configuration (P1)**
*Goal*: Allow runtime modification of rendering parameters via the debug settings interface. Requires T012, T021.

- [ ] T030 Update the main GUI to include a dedicated "Debug Controls" panel component (imGUI). Path: `src/ui/debug_panel.cpp`
- [P] T031 Implement logic in Debug Panel to read and write parameters via the Central Core Manager API (`IRenderConfigurator`). Path: `src/renderer/debug_controls.cpp`
- [ ] T032 Update the render loop (T021) to consume settings from the Central Core Manager *before* drawing the geometry, ensuring changes are reflected immediately. Path: `src/renderer/basic_pass.cpp`

### **[US3] Live Graphics Statistics View (P2)**
*Goal*: Display FPS, GPU Compute Util., VRAM Usage, RAM Usage in real-time within the debug view. Requires OS interaction and threading.

- [ ] T040 Implement a platform abstraction layer to read system metrics (`nvidia-smi` wrapper). Path: `src/system_metrics/gpu_monitor.cpp`
- [P] T041 Create the Statistics Dashboard component (imGUI) that polls the monitor and displays structured data. Path: `src/ui/stats_dashboard.cpp`
- [ ] T042 Integrate system metric polling into a dedicated, low-frequency background thread managed by the Core Manager. Path: `src/core/background_scheduler.cpp`

### **[US4] Plugin Extensibility Layer (P3)**
*Goal*: Allow external modules to register and interact with the renderer. Requires T010, T004.

- [ ] T050 Implement the mechanism for plugin discovery and loading at runtime within the Core Manager. Path: `src/core/plugin_manager.cpp`
- [P] T051 Create a sample "Hello World" plugin implementation adhering strictly to `IRenderConfigurator`. Path: `plugins/sample_hello_world/PluginA.cpp`
- [ ] Implement the RPC message handling for configuration updates, allowing plugins to call `CentralCoreManager::setParameter()`. Path: `src/rpc/config_handler.cpp`

## Final Phase: Polish & Cross-Cutting Concerns (Polish)

This phase completes all cross-cutting concerns and solidifies the application into a cohesive product.

- [ ] T060 Conduct comprehensive end-to-end integration testing, verifying all P1, P2, and P3 features work together seamlessly using the Message Bus. Path: `tests/integration/full_e2e.cpp`
- [P] T061 Implement robust error handling and graceful degradation for all system resource failures (Vulkan context loss, GPU disconnect). Paths: Various

## Additional Tasks & Hardening

- [ ] T070 [P] Test CentralCoreManager for concurrent state conflicts when two plugins attempt to write conflicting values (e.g., setting the same debug parameter). Path: `tests/core/test_plugin_conflict.cpp`
