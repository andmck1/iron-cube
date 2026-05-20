# Research: Render Triangle

## Decision: Slang Shader Compiler Integration

**Decision**: Use Slang v2025.5 via CMake FetchContent. Compile `.slang` files to SPIR-V at build time using `slangc` executable.

**Rationale**: Slang natively outputs SPIR-V (`-target spirv`) without an intermediate GLSL step. Build-time compilation catches shader errors early and avoids runtime compilation overhead. FetchContent matches the existing CMake dependency pattern. Slang's deterministic descriptor binding eliminates manual `layout(binding=N)` annotations.

**Alternatives considered**:
- Runtime Slang API compilation — rejected: adds ~20MB dependency and complexity for prototype
- Pre-compiled SPIR-V in repo — rejected: loses build-time shader validation
- GLSL with glslc — rejected: Slang provides better type safety and abstraction

**Key details**:
- Command: `slangc shader.slang -profile vulkan1.3 -target spirv -entry main -stage vertex -o output.spv`
- Entry points use `[shader("vertex")]` and `[shader("fragment")]` attributes
- Vulkan 1.3 profile matches project's Vulkan headers v1.4.309

## Decision: Vulkan Rendering Pipeline Architecture

**Decision**: Implement `RendererVulkan` class owning the full Vulkan pipeline: instance, physical device, logical device, surface, swap chain, render pass, framebuffers, render pipeline, command buffers, and synchronization primitives.

**Rationale**: Vulkan requires explicit management of all pipeline components. A single class encapsulating the pipeline keeps related resources together and simplifies initialization order and cleanup. The class implements the existing `IRenderer` interface.

**Alternatives considered**:
- Split Vulkan components into separate classes — rejected: initialization ordering becomes fragile
- Use a Vulkan wrapper library (e.g., Vulkan-Hpp only) — rejected: project uses native Vulkan C API with C++ wrappers for error handling

**Pipeline components in order**:
1. VkInstance with GLFW surface extensions + validation layers (Debug)
2. VkPhysicalDevice selection (discrete GPU preferred)
3. VkDevice with graphics queue family + swapchain extension
4. VkSurface from GLFW window
5. Swap chain with B8G8R8A8_SRGB format
6. Render pass with single color attachment (clear to black)
7. Framebuffers (one per swap chain image)
8. Graphics pipeline with hardcoded triangle vertices in vertex shader
9. Command buffer recording: begin render pass → bind pipeline → draw(3) → end render pass
10. Synchronization: image-available semaphore, render-complete semaphore, fence per frame

## Decision: Vulkan Error Handling Pattern

**Decision**: `vkCheck(VkResult, const char*)` inline function that throws `std::runtime_error` on failure. Validation layers enabled in Debug builds. Debug utils messenger callback for warnings and errors.

**Rationale**: Centralizes repetitive VkResult checking. Exceptions propagate to Application::run() which catches and logs via spdlog before clean shutdown. Validation layers catch illegal state during development.

**Alternatives considered**:
- Return codes throughout — rejected: inconsistent with C++20 idioms and existing code
- expected<T> wrapper — rejected: overkill for prototype scope

## Decision: Window Resize Handling

**Decision**: GLFW framebuffer resize callback triggers `vkDeviceWaitIdle` → destroy swap chain, framebuffers, image views → recreate with new extent → continue rendering.

**Rationale**: Vulkan requires swap chain recreation on resize. Waiting for idle ensures no in-flight command references exist. This is the standard pattern from Vulkan tutorials and handles the FR-007 requirement.

**Alternatives considered**:
- Full renderer recreate — rejected: wasteful; only swap chain depends on dimensions
- Queue fence wait instead of device wait idle — rejected: device wait idle is simpler for single-queue setup

## Decision: Minimized Window Handling

**Decision**: Check `glfwGetWindowAttrib(GLFW_ICONIFIED)` each frame. Skip render and present when iconified.

**Rationale**: Vulkan swap chain operations fail on iconified windows. Checking each frame is simple and avoids unnecessary GPU work. Satisfies FR-008 and FR-009.

**Alternatives considered**:
- GLFW iconify callback — rejected: polling aligns with existing main loop pattern

## Decision: Scene Data Structure

**Decision**: AoS (Array of Structures) for the triangle's 3 vertices: `struct Vertex { glm::vec2 pos; glm::vec3 color; }`. Design for SoA transition when scene grows beyond prototype.

**Rationale**: Three vertices is trivial for cache efficiency. AoS matches Vulkan's expected vertex buffer format directly. The `SceneData` structure uses `std::vector<Vertex>` allowing easy extension. Data-oriented principle is satisfied by keeping data in contiguous vectors rather than scene object hierarchies.

**Alternatives considered**:
- SoA from start — rejected: premature optimization for 3 vertices; adds interleaving code for GPU upload
- Hardcoded in shader only — rejected: FR-004 and FR-005 require procedural generation and scene data storage

## Decision: Resource Management Approach

**Decision**: Simple `IResourceManager` service that tracks VkDevice, VkDeviceMemory allocations, and provides `allocateBuffer()`, `allocateImage()`, and `cleanup()` methods. Resources tracked in a vector for batch cleanup.

**Rationale**: Service-locator pattern requires interface-based resource access. A simple tracker avoids manual leak-prone cleanup. The renderer queries the resource manager for buffer allocation rather than managing VkDeviceMemory directly.

**Alternatives considered**:
- RAII wrappers for each Vulkan object — rejected: adds indirection; tracker is simpler for prototype
- Vulkan allocator library (e.g., VMA) — rejected: overkill for single triangle; adds dependency

## Decision: GLM Mathematics Library

**Decision**: Use GLM (OpenGL Mathematics) for vector/matrix types. Already present as project dependency from Feature 003.

**Rationale**: GLM provides `glm::vec2`, `glm::vec3`, `glm::mat4` types that match Vulkan's expected memory layout. Header-only library with no build integration needed. Widely used in graphics programming with familiar SIMD-friendly layout.

**Alternatives considered**:
- Custom math types — rejected: unnecessary duplication; GLM is battle-tested
- Eigen — rejected: heavier dependency; overkill for 2D/3D vector math
