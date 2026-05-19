# Quick Start: Linux App Window

## Prerequisites

- Linux with X11 or Wayland display server
- CMake 4.0.2+
- Ninja build system
- GCC 13+ or Clang 15+
- Vulkan SDK (for headers at build time)

## Build

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Run

```bash
./build/bin/iron-cube
```

A 1280x720 window titled "Iron Cube" should appear. The application runs until you either:
- Click the window close button, or
- Press the Escape key (while the window is visible)

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Expected Behavior

| Action | Result |
|--------|--------|
| Launch | Window appears within 3 seconds |
| Close button | Application exits cleanly (return code 0) |
| Escape key (visible) | Application exits cleanly (return code 0) |
| Escape key (minimized) | Application continues running |
| Other keys | No effect, application continues |
| No display server | Error to stderr, non-zero exit code |
