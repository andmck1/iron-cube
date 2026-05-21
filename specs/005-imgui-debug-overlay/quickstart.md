# Quickstart: ImGui Debug Overlay

## Prerequisites

- Feature 004 (Render Triangle) is implemented and working
- Vulkan-capable GPU with drivers installed
- ImGui v1.90.9 already fetched via CMake FetchContent

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

No new system dependencies are required — ImGui is already a FetchContent dependency.

## Run

```bash
./build/src/app/iron-cube
```

On launch, you should see:
1. A red triangle centered in the window
2. An ImGui debug overlay window in the upper-left corner showing:
   - Current FPS counter
   - Position X/Y sliders
   - Rotation slider
   - Scale slider
   - Reset button

## Verify Debug Overlay

1. **FPS counter**: Observe the FPS value — it should update each frame and reflect your display's refresh rate (typically 60 fps)
2. **Position controls**: Drag the Position X/Y sliders — the triangle should move in real-time
3. **Rotation control**: Drag the Rotation slider — the triangle should rotate around its center
4. **Scale control**: Drag the Scale slider — the triangle should grow or shrink
5. **Reset button**: Click Reset — the triangle should return to its default centered position with no rotation and unit scale

## Architecture Overview

The debug overlay is implemented as a separate layer in the rendering stack:

```
Application::run()
  ├── Layer Stack iteration:
  │     ├── SceneLayer::render()  → draws triangle via VulkanRenderer
  │     └── DebugLayer::render()  → draws ImGui overlay via VulkanRenderer
  └── Swap buffers
```

Each layer implements `ILayer` with `initialize()`, `render()`, and `shutdown()` methods. The Application manages the layer stack and iterates through layers each frame.

## Key Files

| File | Purpose |
|------|---------|
| `src/core/src/debug/ILayer.h` | Common layer interface |
| `src/core/src/debug/DebugLayer.h/cpp` | ImGui debug overlay implementation |
| `src/core/src/rendering/IRenderer.h` | Extended with ImGui hooks |
| `src/core/src/rendering/linux/VulkanRenderer.h/cpp` | ImGui Vulkan backend integration |
| `src/core/src/scene/IScene.h` | Extended with transform accessors |
| `src/core/src/scene/SceneData.h` | Extended with TriangleTransform |
| `src/app/src/Application.h/cpp` | Extended with layer stack management |
