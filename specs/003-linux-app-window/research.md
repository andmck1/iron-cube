# Research: Linux App Window

## Decision: Platform Abstraction Pattern

**Decision**: Interface-based platform layer with GLFW backend, consumed via service locator.

**Rationale**: Constitution Principle V requires service-locator pattern. Interfaces (IPlatform, IWindow, IInput) allow future multi-platform support while keeping the application layer decoupled from GLFW details.

**Alternatives considered**:
- Direct GLFW calls in main.cpp — rejected: violates layered architecture and service-locator principles
- Factory pattern instead of service locator — rejected: constitution mandates service-locator pattern
- Platform-specific compilation units only — rejected: interfaces needed for testability and future extension

## Decision: GLFW Event Processing Model

**Decision**: Use `glfwPollEvents()` in a while-loop driven by window close flag and key state.

**Rationale**: GLFW's polling model is the standard pattern for game/engine main loops. It's synchronous, predictable, and works well with the service-locator pattern since the platform service exposes `pollEvents()` as a method.

**Alternatives considered**:
- Callback-only model — rejected: callbacks don't integrate cleanly with service-locator; state must be polled by application layer
- `glfwWaitEvents()` — rejected: blocks when no events; polling is simpler for an engine that will later add rendering

## Decision: Window Close Handling

**Decision**: Use GLFW's `glfwSetWindowCloseCallback` to set a close flag, checked each frame in the main loop.

**Rationale**: The close callback sets a boolean flag that the main loop checks. This keeps the exit path controlled by the application layer rather than being forced by the callback.

**Alternatives considered**:
- Return immediately from close callback — rejected: bypasses application layer cleanup
- Set window shouldClose via `glfwSetWindowShouldClose` from app — rejected: redundant with callback flag

## Decision: Keyboard Input Handling

**Decision**: Use GLFW's `glfwSetKeyCallback` to track key state in a key-state array. The platform service exposes a `isKeyPressed()` query method.

**Rationale**: Storing key state in the platform layer allows the application to query key state each frame. This is the standard game-engine pattern and integrates with the polling model.

**Alternatives considered**:
- Direct callback to application exit — rejected: bypasses platform abstraction layer
- Event queue — rejected: over-engineered for single-key handling; key-state array is simpler

## Decision: Minimized Window Detection

**Decision**: Track window iconified state via `glfwGetWindowAttrib(window, GLFW_ICONIFIED)` queried each frame.

**Rationale**: GLFW provides this attribute directly. The platform layer exposes it as a method, allowing the application to check if Escape should be ignored.

**Alternatives considered**:
- GLFW iconify callback — rejected: polling aligns with main loop pattern; callback adds unnecessary complexity

## Decision: Error Handling for Window Creation Failure

**Decision**: Log error via spdlog, print to stderr, return non-zero exit code.

**Rationale**: Spec FR-010 and FR-011 require stderr output and non-zero exit. spdlog is already available and configured for logging.

**Alternatives considered**:
- Silent exit — rejected: spec requires error message
- Retry logic — rejected: no reasonable retry for display server absence
