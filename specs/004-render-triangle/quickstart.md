# Quickstart: Render Triangle

## Prerequisites

- Linux system with Vulkan-capable GPU and drivers installed
- Vulkan validation layers installed (`vulkan-validationlayers` package on most distros)
- CMake 4.0.2 or later
- C++20-compatible compiler (GCC 11+, Clang 14+)
- Doxygen (for documentation generation)

Verify Vulkan support:
```bash
vulkaninfo --summary
```

## Build

```bash
cd /home/dtaunt/dev/iron-cube
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
```

The build compiles Slang shaders to SPIR-V, builds the iron-core library, and links the iron-cube executable.

## Run

```bash
./bin/iron-cube
```

**Expected output**:
- Application window titled "Iron Cube" (1280x720)
- Black background with a bright red triangle centered on screen
- Window resize maintains correct triangle rendering
- Press Escape or close window to exit

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Generate Documentation

```bash
cd build
cmake --build . --target ironcube_docs
# Output: build/docs/html/index.html
```

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| "Platform initialization failed" | GLFW cannot connect to display server | Ensure X11 or Wayland session is running |
| "Renderer initialization failed" | No Vulkan-capable GPU or missing drivers | Install GPU drivers and Vulkan ICD |
| "Shader compilation failed" | Slang compiler not found | Verify CMake build completed successfully |
| Segfault on launch | Missing validation layers (Debug) | Install `vulkan-validationlayers` package |
| Black window, no triangle | Shader mismatch or pipeline error | Check spdlog output for Vulkan errors |
