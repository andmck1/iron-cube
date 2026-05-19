# Quickstart: Building Iron Cube

## Prerequisites

- **CMake** 4.0.2 or later
- **GCC** with C++20 support (GCC 11+)
- **Ninja** build system
- **Doxygen** (optional, for documentation generation)
- **Vulkan loader** installed on system (`libvulkan1` package)

## Build

```bash
# Configure (Ninja backend, Debug build)
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

The configuration step automatically fetches all third-party dependencies via CMake FetchContent.

## Run

```bash
./build/bin/iron-cube
```

## Expected Output

```
hello world
```

## Run Tests

```bash
# Run all tests
ctest --test-dir build

# Run with verbose output
ctest --test-dir build --verbose
```

## Build Types

| Type | Description |
|------|-------------|
| Debug | DWARF-4 debug symbols, no optimization (`-g -gdwarf-4 -O0`) |
| Release | Optimized build, stripped debug info |

## Generate Documentation

```bash
# Generate Doxygen documentation
doxygen docs/Doxyfile
```

## Troubleshooting

- **CMake not found**: Install CMake 4.0.2+ from cmake.org or your package manager
- **GCC too old**: Ensure C++20 support (GCC 11+)
- **Ninja not found**: Install via your package manager (`ninja-build` on Debian/Ubuntu)
- **FetchContent download fails**: Check network connectivity; delete `build/` and reconfigure
- **Vulkan loader missing**: Install `vulkan-loader` package on your distribution
