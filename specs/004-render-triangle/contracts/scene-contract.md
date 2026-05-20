# Scene Service Contract

## Interface: IScene

**Location**: `src/core/src/scene/IScene.h`

### Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| initialize | `bool initialize()` | Procedurally generate triangle geometry data. Returns true on success. |
| getVertices | `const std::vector<Vertex>& getVertices() const` | Return reference to vertex data array. |
| getVertexCount | `uint32_t getVertexCount() const` | Return number of vertices in scene. |
| shutdown | `void shutdown()` | Clear all scene data. |

### Data Structures

```cpp
struct Vertex {
    glm::vec2 pos;    // Normalized device coordinates
    glm::vec3 color;  // RGB, range [0.0, 1.0]
};
```

### Behavior

- `initialize()` creates exactly 3 vertices forming a triangle
- Triangle is centered, pointing up, with bright red color (1.0, 0.0, 0.0)
- Vertex data is stored contiguously in a `std::vector`
- Data is read-only after initialization (renderer consumes via const reference)

### Triangle Geometry

| Vertex | Position (x, y) | Color (R, G, B) |
|--------|-----------------|-----------------|
| 0 | (-0.5, -0.5) | (1.0, 0.0, 0.0) |
| 1 | (0.5, -0.5) | (1.0, 0.0, 0.0) |
| 2 | (0.0, 0.5) | (1.0, 0.0, 0.0) |
