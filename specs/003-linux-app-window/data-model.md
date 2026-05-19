# Data Model: Linux App Window

## Entities

### WindowState

Represents the current state of the application window.

| Field | Type | Description |
|-------|------|-------------|
| title | string | Window title ("Iron Cube") |
| width | uint32 | Window width in pixels (default: 1280) |
| height | uint32 | Window height in pixels (default: 720) |
| is_open | bool | Whether the window is currently visible |
| is_iconified | bool | Whether the window is minimized |
| should_close | bool | Whether the user has requested window close |

**Validation**: width and height must be positive values.

### KeyEvent

Represents a keyboard input event captured by the platform layer.

| Field | Type | Description |
|-------|------|-------------|
| key_code | int32 | GLFW key identifier (e.g., GLFW_KEY_ESCAPE) |
| action | enum | Press, Release, Repeat |

**Validation**: key_code maps to a valid GLFW key constant.

### PlatformConfig

Configuration for platform initialization.

| Field | Type | Description |
|-------|------|-------------|
| window_title | string | Title for the application window |
| window_width | uint32 | Initial window width |
| window_height | uint32 | Initial window height |

**Defaults**: title="Iron Cube", width=1280, height=720.

## State Transitions

### Window Lifecycle

```
Created → Visible → (normal operation)
              ↓
         Iconified ←→ Visible
              ↓
         Closing → Destroyed
```

- **Created**: Window object exists, GLFW window handle obtained
- **Visible**: Window is displayed on screen
- **Iconified**: Window is minimized by user
- **Closing**: Close flag set (close button or Escape key), main loop winding down
- **Destroyed**: Window destroyed, resources freed

### Input State

```
Key Released → Key Pressed → Key Released
```

- Key state is tracked as binary (pressed/not pressed) in the platform layer
- Repeat events are ignored for Exit functionality
