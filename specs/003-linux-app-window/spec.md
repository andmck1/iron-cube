# Feature Specification: Linux App Window

**Feature Branch**: `003-linux-app-window`

**Created**: 2026-05-19

**Status**: Draft

## Clarifications

### Session 2026-05-19

- Q: How should the application behave when it cannot create a window? → A: Print error message to stderr and exit with non-zero code
- Q: What are the default window dimensions? → A: 1280x720
- Q: What happens if the window is minimized and Escape is pressed? → A: Escape is ignored while minimized

**Input**: User description: "We are now going to create the next feature for our graphics renderer for developers: Iron Cube. In this feature, which will be called 'Linux App Window'. This feature will be considered complete when the app uses glfw to load up an app window which can be closed or Escape pressed to exit. This feature will focus on the platform abstraction layer of our architecture (window and input) and the application layer (main loop)."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Launch Application Window (Priority: P1)

As a developer using Iron Cube, I want the application to open a window on my Linux desktop when I run it, so I have a visual surface to render graphics to.

**Why this priority**: This is the foundational capability — without a window, there is no graphics renderer.

**Independent Test**: Can be fully tested by running the Iron Cube executable and verifying a window appears on screen.

**Acceptance Scenarios**:

1. **Given** Iron Cube is built and executable, **When** I run the application, **Then** a window appears on my desktop with a visible title identifying it as Iron Cube
2. **Given** the application window is open, **When** I close the window using my window manager's close button, **Then** the application exits cleanly

---

### User Story 2 - Keyboard Exit (Priority: P1)

As a developer using Iron Cube, I want to be able to press Escape to close the application, so I have a keyboard shortcut to exit without using the mouse.

**Why this priority**: Keyboard exit is a standard expectation for desktop applications and provides an alternative exit path.

**Independent Test**: Can be fully tested by running the application, pressing Escape, and verifying the application exits.

**Acceptance Scenarios**:

1. **Given** the application window is open and focused, **When** I press the Escape key, **Then** the application closes the window and exits cleanly
2. **Given** the application window is open, **When** I press any key other than Escape, **Then** the application continues running normally
3. **Given** the application window is minimized, **When** I press the Escape key, **Then** the application continues running without exiting

---

### User Story 3 - Application Main Loop (Priority: P1)

As a developer using Iron Cube, I expect the application to remain responsive in a continuous loop, so the window stays open and processes input until I choose to exit.

**Why this priority**: A main loop is essential for any graphics application to remain alive and process events.

**Independent Test**: Can be fully tested by running the application and verifying it stays open and responsive until an exit action is taken.

**Acceptance Scenarios**:

1. **Given** the application is running, **When** no exit action is taken, **Then** the window remains open and responsive
2. **Given** the application is running, **When** I move or resize the window via my window manager, **Then** the application continues running without crashing

---

### Edge Cases

- When the system lacks a compatible display server, the application prints an error to stderr and exits with a non-zero return code
- When launched without a graphical session, the application prints an error to stderr and exits with a non-zero return code
- When the window is minimized and Escape is pressed, the application ignores the key press and continues running

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST create a visible application window with 1280x720 dimensions when the Iron Cube executable starts
- **FR-002**: System MUST display a window title that identifies the application as Iron Cube
- **FR-003**: System MUST run a continuous main loop that keeps the application alive and processing events
- **FR-004**: System MUST close the window and exit cleanly when the user clicks the window close button
- **FR-005**: System MUST close the window and exit cleanly when the user presses the Escape key while the window is visible
- **FR-006**: System MUST ignore the Escape key press when the window is minimized
- **FR-007**: System MUST ignore all key presses other than Escape and continue running normally
- **FR-008**: System MUST handle window manager events (minimize, restore, resize) without crashing
- **FR-009**: System MUST exit with a clean return code indicating successful termination
- **FR-010**: System MUST print an error message to stderr when window creation fails
- **FR-011**: System MUST exit with a non-zero return code when window creation fails

### Key Entities

- **Application Window**: The primary visual surface managed by the platform layer. Represents the visible window on the desktop with properties like title, dimensions, and lifecycle state.
- **Input Event**: A discrete signal from user input devices (keyboard, mouse) that the platform layer captures and routes to the application for processing.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Application window appears within 3 seconds of launching the executable
- **SC-002**: Window close button results in application exit within 1 second of being clicked
- **SC-003**: Pressing Escape when the window is visible results in application exit within 1 second
- **SC-004**: Pressing Escape when the window is minimized has no effect and the application continues running
- **SC-005**: Application exits with return code 0 on clean shutdown
- **SC-006**: Application remains stable and responsive for at least 5 minutes of continuous operation with no user interaction

## Assumptions

- Target platform is Linux with a compatible display server (X11 or Wayland)
- GLFW 3.4 is available as a dependency (already configured in the build system)
- The application runs in a graphical desktop environment, not a headless server
- Default window size is 1280x720; position is chosen by the window manager
- The window background color does not matter for this feature (any solid color is acceptable)
- No rendering or graphics output is required beyond having a functional window
- The platform abstraction layer will be designed to allow future extension (Vulkan context, additional input, etc.)
- spdlog is available for logging initialization and shutdown messages
