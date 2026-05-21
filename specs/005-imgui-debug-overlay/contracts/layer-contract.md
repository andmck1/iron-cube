# Layer Contract

## Overview

`ILayer` defines the common interface for renderable components in the layer stack. Each layer is responsible for its own initialization, per-frame rendering, and cleanup.

## Interface

```cpp
class ILayer
{
public:
    virtual ~ILayer() = default;

    /**
     * @brief Initialize layer resources.
     * Called once during application startup before the render loop.
     * @return true on success.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Render this layer's content for the current frame.
     * Called once per frame in stack order.
     */
    virtual void render() = 0;

    /**
     * @brief Release layer resources.
     * Called during application shutdown in reverse stack order.
     */
    virtual void shutdown() = 0;
};
```

## Lifecycle

1. **Creation**: Application constructs layer instances
2. **Initialization**: `initialize()` called in stack order (scene before debug)
3. **Render loop**: `render()` called each frame in stack order
4. **Shutdown**: `shutdown()` called in reverse stack order (debug before scene)

## Thread Safety

All methods must be called from the main thread.

## Layer Ordering

Layers are stored in a `std::vector<std::unique_ptr<ILayer>>` and rendered in insertion order. The default stack is:

| Order | Layer | Purpose |
|-------|-------|---------|
| 0 | SceneLayer | Renders scene geometry (triangle) |
| 1 | DebugLayer | Renders ImGui debug overlay |

## Design Notes

- Layers access services (IRenderer, IScene, etc.) through the ServiceLocator
- Layers do not reference each other directly
- Adding a new layer requires only constructing it and pushing it onto the stack
