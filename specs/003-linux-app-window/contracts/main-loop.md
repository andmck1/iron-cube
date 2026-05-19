# Main Loop Contract

## Overview

The application main loop in `main.cpp` orchestrates platform initialization, event processing, and shutdown.

## Sequence

```
1. spdlog initialization
2. Service locator setup
3. Platform registration
4. Platform::initialize()
5. Platform::createWindow(config)
6. While !Platform::shouldExit():
     Platform::pollEvents()
     // Future: render frame
7. Platform::shutdown()
8. Return 0
```

## Error Paths

| Condition | Behavior |
|-----------|----------|
| initialize() fails | Log error, exit non-zero |
| createWindow() fails | Log error, exit non-zero |
| Normal exit | Clean shutdown, exit 0 |
