# Debug Layer Contract

## Overview

`DebugLayer` implements `ILayer` to provide an ImGui-based debug overlay with FPS monitoring and triangle transform controls.

## Interface

Implements `ILayer` with the following behavior:

### `initialize()`

- Assumes ImGui context already created by VulkanRenderer
- Initializes ImGui style (dark theme)
- Reads initial transform values from `IScene`
- **Returns**: true on success

### `render()`

Per-frame execution:
1. Reads current FPS value (provided by Application)
2. Reads current triangle transform from `IScene::getTriangleTransform()`
3. Renders ImGui window with:
   - FPS counter display
   - Position X/Y sliders (range: -5.0 to 5.0, step: 0.01)
   - Rotation slider (range: 0° to 360°, step: 1.0)
   - Scale slider (range: 0.1 to 10.0, step: 0.1)
   - Reset button
4. Writes modified transform to `IScene::setTriangleTransform()` if values changed

### `shutdown()`

- Cleans up layer-specific resources
- Does NOT destroy ImGui context (owned by VulkanRenderer)

## Dependencies

| Service | Access Method | Purpose |
|---------|---------------|---------|
| `IScene` | `ServiceLocator::get<IScene>()` | Read/write triangle transform |

**Note**: DebugLayer calls the ImGui C API directly (`ImGui::Begin`, `ImGui::SliderFloat`, etc.). The renderer executes ImGui draw commands via `renderImGui()` after the layer's `render()` returns. DebugLayer has no dependency on `IRenderer`.

## UI Layout

```
+---------------------------+
| Triangle Debug [x]        |
+---------------------------+
| FPS: 60                   |
+---------------------------+
| Position X: [-0.50]       |
| Position Y: [ 0.00]       |
| Rotation:   [  0.0]°      |
| Scale:      [  1.0]       |
+---------------------------+
|         [Reset]           |
+---------------------------+
```

## Edge Case Behavior

- **Scale = 0**: Clamped to minimum 0.1
- **Position out of range**: Clamped to [-5.0, 5.0]
- **Reset**: Sets position to (0,0), rotation to 0°, scale to 1.0
