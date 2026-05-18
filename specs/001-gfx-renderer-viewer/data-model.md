# Data Model Specification: GFX Renderer Viewer

**Date**: 2026-05-18
**Context**: Defines data structures and state required by the renderer.

## Key Entities

### 1. Render Target (View)
*   **Purpose**: The output surface for all rendered geometry.
*   **Attributes**:
    *   `width`: Integer (Pixel dimensions).
    *   `height`: Integer (Pixel dimensions).
    *   `format`: Enum (e.g., RGBA8, Depth/Stencil format).
    *   `coordinate_system`: String ("Screen Space" or "Normalized [0,1]").
*   **Relationships**: Is the primary consumer of all rendered data.

### 2. Debug Settings (Core State)
*   **Purpose**: Global, mutable parameters that control renderer behavior for debugging.
*   **Attributes**:
    *   `triangle_color`: Color struct {R: float [0-1], G: float [0-1], B: float [0-1]}
    *   `vertex_scale`: Float (Scaling factor applied to vertex positions).
    *   `is_debug_enabled`: Boolean.
*   **Constraints**: Must be managed by the Central Core Manager and validated against acceptable ranges before applying changes.

### 3. Graphics Statistics Snapshot
*   **Purpose**: Time-series data captured per frame for monitoring performance.
*   **Attributes**:
    *   `timestamp_ms`: Long integer (Time of snapshot).
    *   `frame_time_us`: Integer (Total time taken to render the current frame in microseconds).
    *   `fps`: Float (Calculated FPS value).
    *   `gpu_utilization_%`: Percentage float.
    *   `vram_usage_bytes`: Long integer (Current GPU memory footprint).
    *   `ram_usage_bytes`: Long integer (System RAM usage delta since last check).

### 4. Plugin Configuration
*   **Purpose**: Defines the structure of configuration data passed into an external plugin module.
*   **Attributes**: Schema must be versioned and include a `plugin_id: string` and `version: int`.

## State Transitions & Ownership
The Debug Settings entity is owned exclusively by the Central Core Manager, which guarantees thread-safe read/write access for all other components via its public API.