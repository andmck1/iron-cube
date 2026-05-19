# Data Model: Build System

## Summary

This feature does not introduce scene data or domain entities. The data model section documents the build configuration structure, dependency graph, and planned entities for future phases.

## Build Configuration Entities

### Project Metadata

| Field | Type | Value |
|-------|------|-------|
| Project name | String | "Iron Cube" |
| C++ standard | Integer | 20 |
| Minimum CMake version | SemVer | 4.0.2 |
| Languages | List | CXX |
| Compiler | String | GCC |
| Build Backend | String | Ninja |
| Target Platform | String | Linux |

### Build Artifacts

| Artifact | Type | Path |
|----------|------|------|
| Application binary | Executable | `${CMAKE_BINARY_DIR}/bin/iron-cube` |
| Archive libraries | Static lib | `${CMAKE_BINARY_DIR}/lib/` |
| Shared libraries | Dynamic lib | `${CMAKE_BINARY_DIR}/lib/` |
| Test binaries | Executable | `${CMAKE_BINARY_DIR}/bin/` |

### Source Layout

| Component | Role | Path |
|-----------|------|------|
| app | Executable entry point | `src/app/` |
| core | Shared library (stubbed) | `src/core/` |
| tests | Catch2 test infrastructure | `tests/` |
| docs | Doxygen configuration | `docs/` |

## Dependency Graph

```
iron-cube (executable)
├── iron-core (static library)
│   ├── GLFW
│   ├── Vulkan::Headers
│   ├── GLM
│   ├── spdlog
│   ├── stb (interface library)
│   └── tinyobjloader (interface library)
├── Dear ImGui
│   ├── GLFW (shared)
│   └── Vulkan::Headers (shared)
└── Catch2 (test only)
```

### Dependency Details

| Dependency | Type | Link Mode | Notes |
|------------|------|-----------|-------|
| GLFW | Library | Linked | Window/input, shared with ImGui |
| Dear ImGui | Library | Linked | Immediate-mode GUI |
| spdlog | Library | Linked | Logging |
| GLM | Header-only | Interface | Math library |
| stb | Header-only | Interface | Image/font utilities |
| tinyobjloader | Header-only | Interface | OBJ model loading |
| Vulkan Headers | Headers | Interface | API definitions |
| Catch2 | Library | Linked (tests) | Unit testing |
| Doxygen | Tool | N/A | Doc generation (optional) |

## Validation Rules

- CMake configuration MUST succeed before any compilation
- All FetchContent dependencies MUST resolve without network errors
- Binary output MUST be placed in centralized output directory
- Compile commands MUST be exported for tooling integration
- Catch2 test discovery MUST find at least one test target

## Future Entities (Out of Scope)

The following entities are anticipated for future features and are not part of this phase:

- **Scene**: Container for scene graph data (data-oriented, not class-based)
- **Renderer**: Rendering service interface
- **Window**: Window management service interface
