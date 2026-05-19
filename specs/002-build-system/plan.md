# Implementation Plan: Build System

**Branch**: `002-build-system` | **Date**: 2026-05-19 | **Spec**: [spec.md](./spec.md)

**Input**: Feature specification from `/specs/002-build-system/spec.md`

## Summary

Set up the CMake build system for Iron Cube, a cross-platform graphics application framework. The project must compile on Linux using CMake 4.0.2+, GCC, and Ninja. All third-party dependencies (GLFW, ImGui, spdlog, GLM, stb, tinyobjloader, Vulkan headers, Catch2, Doxygen) are fetched via CMake FetchContent. The build produces a runnable binary that prints "hello world" on execution.

## Technical Context

**Language/Version**: C++20

**Compiler**: GCC (C++20 capable)

**Build Backend**: Ninja

**Primary Dependencies**: GLFW, Dear ImGui, spdlog, GLM, stb, tinyobjloader, Vulkan Headers, Catch2, Doxygen

**Dependency Management**: CMake FetchContent (all dependencies fetched automatically)

**Storage**: N/A

**Testing**: Catch2 unit test framework

**Target Platform**: Linux (Windows deferred)

**Project Type**: desktop-app (graphics application framework)

**Performance Goals**: N/A for this phase

**Constraints**: Out-of-source builds required; compile commands exported; debug builds with DWARF-4 symbols; FetchContent for all third-party deps

**Scale/Scope**: Single application binary, 9 third-party dependencies fetched automatically

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| Principle | Status | Notes |
|-----------|--------|-------|
| I. Test-Driven Development | ✅ Pass | Catch2 test framework included; build verification tests planned |
| II. Documentation First | ✅ Pass | Spec, plan, research, quickstart written before implementation |
| III. Layered & Component-Based | ✅ Pass | src/app (executable) and src/core (library) layers established |
| IV. Data-Oriented Design | ⚠ N/A | No scene data in this phase |
| V. Service-Locator Pattern | ⚠ N/A | No services in this phase |
| VI. Incremental Feature Development | ✅ Pass | Build system is the first increment |

## Project Structure

### Documentation (this feature)

```text
specs/002-build-system/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── tasks.md             # Phase 2 output (created by /speckit.tasks)
```

### Source Code (repository root)

```text
CMakeLists.txt           # Root CMake configuration + FetchContent
src/
├── app/
│   ├── CMakeLists.txt   # Application executable target
│   └── src/
│       └── main.cpp     # Application entry point
└── core/
    └── CMakeLists.txt   # Core library target (stubbed)
tests/
    ├── CMakeLists.txt   # Test infrastructure
    └── build/
        └── test_build.cpp  # Build verification test (Catch2)
docs/
    └── Doxyfile         # Doxygen configuration
```

**Structure Decision**: Single project with layered source layout. `src/app` contains the application entry point and executable target. `src/core` will contain shared library components (stubbed for this phase). `tests/` contains Catch2-based build verification tests. `docs/` holds Doxygen configuration. All third-party dependencies are managed via FetchContent in the root CMakeLists.txt.

## Complexity Tracking

No constitution violations — no complexity tracking needed.
