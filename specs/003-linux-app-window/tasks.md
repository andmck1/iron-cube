# Tasks: Linux App Window

**Input**: Design documents from `/specs/003-linux-app-window/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization — create directory structure and service locator foundation.

- [x] T001 Create `src/core/src/platform/linux/` directory structure for platform interfaces and implementations
- [x] T002 Create `src/core/src/service/` directory for service locator

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core interfaces and service locator that ALL user stories depend on.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [x] T003 Implement `IWindow` interface in `src/core/src/platform/IWindow.h`
- [x] T004 [P] Implement `IInput` interface in `src/core/src/platform/IInput.h`
- [x] T005 [P] Implement `IPlatform` interface in `src/core/src/platform/IPlatform.h`
- [x] T006 Implement `ServiceLocator` header in `src/core/src/service/ServiceLocator.h`
- [x] T007 Implement `ServiceLocator` in `src/core/src/service/ServiceLocator.cpp`
- [x] T008 Update `src/core/CMakeLists.txt` to include new source files and service/ include path

**Checkpoint**: Foundation ready — platform interfaces, service locator, and build system updated.

---

## Phase 3: User Story 1 - Launch Application Window (Priority: P1) 🎯 MVP

**Goal**: Application opens a 1280x720 window titled "Iron Cube" on Linux desktop; close button exits cleanly.

**Independent Test**: Run the executable and verify a window appears; click close button and verify clean exit.

### Tests for User Story 1 ⚠️ TDD

- [x] T009 [P] [US1] Window creation test in `tests/build/test_window.cpp` — verify window can be created and destroyed
- [x] T010 [P] [US1] Platform initialization test in `tests/build/test_platform.cpp` — verify initialize/shutdown cycle

### Implementation for User Story 1

- [x] T011 [P] [US1] Implement `WindowGLFW` header in `src/core/src/platform/linux/WindowGLFW.h`
- [x] T012 [P] [US1] Implement `PlatformLinux` header in `src/core/src/platform/linux/PlatformLinux.h`
- [x] T013 [US1] Implement `WindowGLFW` in `src/core/src/platform/linux/WindowGLFW.cpp` (depends on T011)
- [x] T014 [US1] Implement `PlatformLinux` in `src/core/src/platform/linux/PlatformLinux.cpp` (depends on T012, T013)
- [x] T015 [US1] Add window close callback in `src/core/src/platform/linux/WindowGLFW.cpp` to set close flag

**Checkpoint**: Window creation, display, and close button exit work independently.

---

## Phase 4: User Story 2 - Keyboard Exit (Priority: P1)

**Goal**: Pressing Escape closes the application; Escape is ignored when window is minimized.

**Independent Test**: Run the application, press Escape, verify exit; minimize window, press Escape, verify application continues.

### Tests for User Story 2 ⚠️ TDD

- [x] T016 [P] [US2] Input key state test in `tests/build/test_input.cpp` — verify key press/release tracking

### Implementation for User Story 2

- [x] T017 [P] [US2] Implement `InputGLFW` header in `src/core/src/platform/linux/InputGLFW.h`
- [x] T018 [US2] Implement `InputGLFW` in `src/core/src/platform/linux/InputGLFW.cpp` (depends on T017)
- [x] T019 [US2] Integrate `InputGLFW` into `PlatformLinux` in `src/core/src/platform/linux/PlatformLinux.cpp`
- [x] T020 [US2] Implement `shouldExit()` logic in `src/core/src/platform/linux/PlatformLinux.cpp` — check close flag OR (Escape pressed AND not iconified)

**Checkpoint**: Keyboard exit and minimized-window Escape ignore work independently.

---

## Phase 5: User Story 3 - Application Main Loop (Priority: P1)

**Goal**: Application runs a continuous event-processing loop, stays responsive, and exits cleanly on user action.

**Independent Test**: Run the application and verify it stays open and responsive until exit action is taken.

### Implementation for User Story 3

- [x] T021 [US3] Update `src/app/src/main.cpp` with spdlog initialization and service locator setup
- [x] T022 [US3] Implement main loop in `src/app/src/main.cpp` — register platform, initialize, create window, poll events, shutdown
- [x] T023 [US3] Implement error handling in `src/app/src/main.cpp` — log to stderr and exit non-zero on init/window failure

**Checkpoint**: Full main loop: init → create → poll → exit → shutdown.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Build validation and integration testing.

- [x] T024 Update `tests/build/CMakeLists.txt` to include new test executables
- [x] T025 [P] Update `src/core/CMakeLists.txt` to include all new platform source files
- [x] T026 Run full build and verify no compilation errors
- [x] T027 Run `ctest` and verify all tests pass
- [x] T028 Run `quickstart.md` validation — launch, close button, Escape, minimized Escape, error handling

---

## Phase 7: Rendering Layer Separation (Constitution Principle III)

**Purpose**: Per Constitution Principle III (Layered & Component-Based Architecture), `swapBuffers()` is a rendering concern and must not live in the platform layer. Create a dedicated rendering layer with `IRenderer` interface.

**Layer boundaries**:
- **Platform layer**: window lifecycle, input, event polling
- **Rendering layer**: framebuffer management, swapBuffers
- **Application layer**: orchestrates poll → render → present

### Tests for Rendering Layer ⚠️ TDD

- [ ] T029 [P] Renderer test in `tests/build/test_renderer.cpp` — verify renderer init, swapBuffers, and shutdown
- [ ] T030 [P] Platform regression test in `tests/build/test_platform.cpp` — verify swapBuffers is no longer on IPlatform

### Implementation for Rendering Layer

- [ ] T031 Implement `IRenderer` interface in `src/core/src/rendering/IRenderer.h`
- [ ] T032 Implement `RendererGLFW` header in `src/core/src/rendering/linux/RendererGLFW.h`
- [ ] T033 Implement `RendererGLFW` in `src/core/src/rendering/linux/RendererGLFW.cpp` (depends on T032, uses `WindowGLFW::getGLFWWindow()`)
- [ ] T034 Remove `swapBuffers()` from `IWindow` in `src/core/src/platform/IWindow.h`
- [ ] T035 Remove `swapBuffers()` from `WindowGLFW` in `src/core/src/platform/linux/WindowGLFW.h` and `.cpp`
- [ ] T036 Remove `swapBuffers()` from `IPlatform` in `src/core/src/platform/IPlatform.h`
- [ ] T037 Remove `swapBuffers()` from `PlatformLinux` in `src/core/src/platform/linux/PlatformLinux.h` and `.cpp`
- [ ] T038 Update `src/core/CMakeLists.txt` to include rendering source files and include path
- [ ] T039 Update `src/app/src/main.cpp` to register `IRenderer` and call `renderer->swapBuffers()` in main loop
- [ ] T040 Run full build and verify no compilation errors
- [ ] T041 Run `ctest` and verify all tests pass

**Checkpoint**: Rendering layer is independent, platform layer has no rendering concerns, main loop orchestrates both layers.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — BLOCKS all user stories
- **User Stories (Phase 3-5)**: All depend on Foundational phase completion
  - US1 must complete before US2 (US2 depends on PlatformLinux from US1)
  - US2 must complete before US3 (US3 depends on shouldExit from US2)
  - Sequential execution required: US1 → US2 → US3
- **Polish (Phase 6)**: Depends on all user stories being complete
- **Rendering Layer (Phase 7)**: Depends on Polish — refactors swapBuffers out of platform into new rendering layer

### Within Each User Story

- Tests MUST be written and FAIL before implementation (TDD per Constitution Principle I)
- Headers before implementation files
- Core implementation before integration
- Story complete before moving to next priority

### Rendering Layer (Phase 7)

- Tests written first (T029, T030)
- `IRenderer` interface before implementation (T031 → T032 → T033)
- Platform cleanup after renderer is ready (T034-T037)
- Build system and main loop integration (T038-T039)
- Validation (T040-T041)

- Tests MUST be written and FAIL before implementation (TDD per Constitution Principle I)
- Headers before implementation files
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- T004 and T005 are parallel (IWindow and IInput are independent interfaces)
- T011 and T012 are parallel (WindowGLFW.h and PlatformLinux.h are independent headers)
- T009 and T010 are parallel (independent test files)
- T024 and T025 are parallel (CMakeLists updates in different directories)
- T029 and T030 are parallel (independent test files)
- T034-T037 are parallel (removing swapBuffers from independent files)

---

## Parallel Example: Foundational Phase

```bash
# Launch all interface headers together:
Task: "Implement IWindow interface in src/core/src/platform/IWindow.h"
Task: "Implement IInput interface in src/core/src/platform/IInput.h"
Task: "Implement IPlatform interface in src/core/src/platform/IPlatform.h"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL — blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Window opens, close button works

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Window creation and close → Validate
3. Add User Story 2 → Keyboard exit → Validate
4. Add User Story 3 → Main loop integration → Validate
5. Polish → Build and test validation
6. Rendering Layer → Extract swapBuffers into IRenderer, clean platform layer → Validate

### Sequential Note

Unlike typical multi-story features, these three stories have tight sequential dependencies:
- US2's `shouldExit()` depends on US1's `PlatformLinux`
- US3's main loop depends on US2's complete exit logic
- **Recommended**: Complete US1 → US2 → US3 in order, validating at each checkpoint

---

## Task Summary

| Phase | Task Count | Parallel Opportunities |
|-------|-----------|----------------------|
| Setup | 2 | 0 |
| Foundational | 6 | 3 (T004, T005, T011/T012) |
| US1 - Launch Window | 7 | 4 (T009, T010, T011, T012) |
| US2 - Keyboard Exit | 5 | 2 (T016, T017) |
| US3 - Main Loop | 3 | 0 |
| Polish | 5 | 2 (T024, T025) |
| Rendering Layer | 13 | 3 (T029, T030, T031) |
| **Total** | **41** | **14 parallelizable** |
