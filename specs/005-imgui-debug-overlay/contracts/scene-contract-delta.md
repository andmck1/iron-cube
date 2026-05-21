# IScene Contract Delta

## Changes from Feature 004

The following methods are **added** to `IScene`:

### `getTriangleTransform()`

```cpp
virtual TriangleTransform getTriangleTransform() const = 0;
```

Returns the current triangle transform (position, rotation, scale).

### `setTriangleTransform()`

```cpp
virtual void setTriangleTransform(TriangleTransform transform) = 0;
```

Updates the triangle transform. Marks scene data as dirty.

### `hasTriangleTransformChanged()`

```cpp
virtual bool hasTriangleTransformChanged() const = 0;
```

Returns true if the transform has been modified since the last render frame.

### `markTransformClean()`

```cpp
virtual void markTransformClean() = 0;
```

Marks the transform as clean after the renderer has consumed the new values.

## Existing Methods

All existing methods from Feature 004 remain unchanged:

| Method | Status |
|--------|--------|
| `initialize()` | Unchanged |
| `getVertices()` | Unchanged |
| `getVertexCount()` | Unchanged |
| `isDirty()` | Unchanged (vertex data only) |
| `markClean()` | Unchanged (vertex data only) |
| `shutdown()` | Unchanged |

## New Data Type

`TriangleTransform` is added to `SceneData.h`:

```cpp
struct TriangleTransform
{
    glm::vec2 position;  ///< Offset in NDC (-1 to +1)
    float rotation;      ///< Degrees, clockwise
    float scale;         ///< Uniform scale factor
};
```
