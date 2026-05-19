# Iron Cube

A Vulkan-based rendering engine built with modern C++ practices.

## Architecture

- **Layered architecture** for high-level abstraction
- **Component-based design** prioritizing composition over inheritance
- **Data-oriented design** with efficient scene data storage
- **Service-locator pattern** for dependency management

## Prerequisites

- CMake 3.28+
- Ninja build system
- GCC 13+ (or Clang 15+)
- Vulkan SDK (for runtime)

## Building

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Testing

```bash
ctest --test-dir build --output-on-failure
```
