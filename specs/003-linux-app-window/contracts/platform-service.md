# Platform Service Contract

## Overview

The Platform service provides window management and input handling for the Iron Cube application. It is the primary service registered with the service locator.

## Interface: IPlatform

| Method | Returns | Description |
|--------|---------|-------------|
| `initialize()` | bool | Initialize platform subsystem. Returns false on failure. |
| `createWindow(const PlatformConfig&)` | bool | Create application window with given config. Returns false on failure. |
| `pollEvents()` | void | Process pending window and input events. |
| `shouldExit()` | bool | Check if window should close (close button or Escape pressed). |
| `shutdown()` | void | Destroy window and clean up platform resources. |

## Interface: IWindow

| Method | Returns | Description |
|--------|---------|-------------|
| `isOpen()` | bool | Whether the window is currently open. |
| `isIconified()` | bool | Whether the window is minimized. |
| `requestClose()` | void | Signal that the window should close. |
| `getSize()` | Vec2u | Current window dimensions in pixels. |

## Interface: IInput

| Method | Returns | Description |
|--------|---------|-------------|
| `isKeyPressed(KeyCode)` | bool | Whether the given key is currently pressed. |

## Service Registration

The platform service is registered with the service locator under the `IPlatform` interface type before the application main loop begins.

```cpp
// Pseudocode
serviceLocator.register<IPlatform>(platformImpl);
auto* platform = serviceLocator.get<IPlatform>();
```

## Error Contract

- `initialize()` returning false → log error to stderr, exit with non-zero code
- `createWindow()` returning false → log error to stderr, exit with non-zero code
- No method throws exceptions; errors are reported via return values or logged
