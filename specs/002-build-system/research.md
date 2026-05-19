# Research: Build System

## Decisions

### Build System: CMake with FetchContent (already chosen)

**Decision**: Use CMake 4.0.2+ with FetchContent for dependency management.

**Rationale**: Already present in project root. Constitution specifies CMake 4.0.2 as minimum version. FetchContent eliminates the need for system-installed dependencies, ensuring reproducible builds from a clean checkout.

**Alternatives considered**: vcpkg, Conan, system packages. Rejected due to: additional tool dependency (vcpkg/Conan), system state coupling (packages), and FetchContent being native to CMake with no extra tooling required.

### Compiler: GCC

**Decision**: Use GCC as the primary compiler.

**Rationale**: User-specified requirement. GCC provides excellent C++20 support and integrates well with Ninja on Linux.

**Alternatives considered**: Clang. Noted for future consideration if cross-compiler testing becomes needed.

### Build Backend: Ninja

**Decision**: Use Ninja as the build backend.

**Rationale**: User-specified requirement. Ninja provides faster rebuilds than Make, especially for incremental builds. Its compact build files and parallel execution make it ideal for C++ projects.

### Platform: Linux Only (initially)

**Decision**: Target Linux exclusively for this phase. Windows support deferred.

**Rationale**: User-specified constraint. Reduces initial complexity and allows focus on core build infrastructure.

**Alternatives considered**: Multi-platform from start. Rejected to keep first increment small and focused.

## Dependency Research

### GLFW (Simple Window and Input Library)

**Purpose**: Window creation, input handling, monitor enumeration, and context management for graphics APIs.

**FetchContent Source**: github.com/glfw/glfw (release branch or tag)

**CMake Integration**: Provides `glfw3` target via `find_package` or direct build.

**Version**: Latest stable (3.4+)

### Dear ImGui (Immediate Mode GUI)

**Purpose**: Debug overlays, tooling interfaces, and in-application UI.

**FetchContent Source**: github.com/ocornut/imgui (master or tagged release)

**CMake Integration**: ImGui provides CMake support via its `backends/` directory. Will need to link GLFW and Vulkan backends.

**Version**: Latest stable (1.90+)

### spdlog (Fast C++ Logging)

**Purpose**: High-performance formatted logging with multiple sinks.

**FetchContent Source**: github.com/gabime/spdlog (vX.Y.Z tag)

**CMake Integration**: Provides `spdlog::spdlog` target. Header-only option available but compiled preferred for TDD.

**Version**: Latest stable (1.14+)

### GLM (OpenGL Mathematics)

**Purpose**: Vector, matrix, and transformation math library matching OpenGL/Vulkan conventions.

**FetchContent Source**: github.com/g-truc/glm (X.Y.Z tag)

**CMake Integration**: Header-only library. Provides `glm::glm` target.

**Version**: Latest stable (1.3+)

### stb (Single-Header Libraries)

**Purpose**: Image loading (`stb_image.h`), image writing (`stb_image_write.h`), and TrueType font loading (`stb_truetype.h`).

**FetchContent Source**: github.com/nothings/stb (master)

**CMake Integration**: Header-only collection. Will wrap in CMake interface library for clean target dependencies.

**Version**: Latest master (no formal versioning)

### tinyobjloader

**Purpose**: Wavefront OBJ model file loading for 3D asset import.

**FetchContent Source**: github.com/syoyo/tinyobjloader (master)

**CMake Integration**: Header-only library. Will wrap in CMake interface library.

**Version**: Latest master

### Vulkan Headers

**Purpose**: Vulkan API headers and loader definitions.

**FetchContent Source**: github.com/KhronosGroup/Vulkan-Headers (vX.Y.Z tag)

**CMake Integration**: Provides `Vulkan::Headers` target. Vulkan loader (`libvulkan.so`) expected to be installed on system.

**Version**: Latest stable matching system Vulkan support

### Catch2 (Test Framework)

**Purpose**: Unit testing framework for TDD compliance.

**FetchContent Source**: github.com/catchorg/Catch2 (v3.X tag)

**CMake Integration**: Provides `Catch2::Catch2WithMain` target and `catch_discover_tests()` function.

**Version**: Catch2 v3 (latest stable)

### Doxygen (Documentation Generator)

**Purpose**: API documentation generation from source code comments.

**FetchContent Source**: Not fetched via FetchContent (Doxygen is a build tool, not a library). Configuration file (`Doxyfile`) included in repo. Doxygen expected to be installed on system for documentation builds.

**Alternative**: Could be optional — only required when generating docs, not for application build.

## Best Practices Applied

- CMake `target_*` commands over global `set()` for compiler flags and dependencies
- FetchContent for reproducible, self-contained builds
- Separate executable and library targets for clean dependency management
- Output directory centralization via `CMAKE_RUNTIME_OUTPUT_DIRECTORY`
- Compile commands export enabled for tooling (clangd, cquery, etc.)
- Catch2 integrated for TDD workflow from day one
- Doxygen configuration for documentation-first compliance
