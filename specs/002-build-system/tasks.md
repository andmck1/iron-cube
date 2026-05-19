# Tasks: Build System

**Input**: Design documents from `/specs/002-build-system/`

**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md

**Tests**: TDD approach requested — test tasks included per FR-008 and SC-005.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project directory structure and root CMake configuration.

- [ ] T001 Create source directory structure: `src/app/`, `src/core/`, `tests/build/`, `docs/`
- [ ] T002 Write root CMakeLists.txt with project metadata, C++20 standard, languages, compile_commands.json export, and output directory configuration

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: FetchContent dependency declarations, CMake target definitions, test infrastructure, and Doxygen configuration. All dependencies and build targets must be wired before any user story can compile.

**⚠️ CRITICAL**: No user story work can begin until this phase is complete.

- [ ] T003 [P] Add FetchContent declaration for GLFW (github.com/glfw/glfw, 3.4+) in CMakeLists.txt
- [ ] T004 [P] Add FetchContent declaration for Dear ImGui (github.com/ocornut/imgui, 1.90+) in CMakeLists.txt
- [ ] T005 [P] Add FetchContent declaration for spdlog (github.com/gabime/spdlog, 1.14+) in CMakeLists.txt
- [ ] T006 [P] Add FetchContent declaration for GLM (github.com/g-truc/glm, 1.3+) in CMakeLists.txt
- [ ] T007 [P] Add FetchContent declaration for stb (github.com/nothings/stb, master) in CMakeLists.txt
- [ ] T008 [P] Add FetchContent declaration for tinyobjloader (github.com/syoyo/tinyobjloader, master) in CMakeLists.txt
- [ ] T009 [P] Add FetchContent declaration for Vulkan Headers (github.com/KhronosGroup/Vulkan-Headers) in CMakeLists.txt
- [ ] T010 [P] Add FetchContent declaration for Catch2 (github.com/catchorg/Catch2, v3) in CMakeLists.txt
- [ ] T011 Write `src/core/CMakeLists.txt` defining `iron-core` static library target with dependency links (GLFW, spdlog, GLM, stb, tinyobjloader, Vulkan Headers)
- [ ] T012 Write `src/app/CMakeLists.txt` defining `iron-cube` executable target linking `iron-core` and Dear ImGui
- [ ] T013 Write `tests/CMakeLists.txt` enabling Catch2 test discovery and linking Catch2::Catch2WithMain
- [ ] T014 [P] Write `docs/Doxyfile` with project name, input paths, and HTML output configuration

**Checkpoint**: Foundation ready — CMake configure should succeed, all dependencies fetched, all targets defined.

---

## Phase 3: User Story 1 - Build Project from Source (Priority: P1) 🎯 MVP

**Goal**: A developer can configure and compile the project from a clean checkout on Linux using CMake, GCC, and Ninja.

**Independent Test**: Running `cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build` completes without errors.

### Tests for User Story 1 ⚠️

> **NOTE**: Write this test FIRST, ensure it fails before implementation is complete.

- [ ] T015 [US1] Write build verification test `tests/build/test_build.cpp` using Catch2 to confirm the project compiles and the binary exists at expected output path

### Implementation for User Story 1

- [ ] T016 [US1] Populate root CMakeLists.txt with `add_subdirectory(src/core)` and `add_subdirectory(src/app)` to wire targets
- [ ] T017 [US1] Add `add_subdirectory(tests)` to root CMakeLists.txt for test discovery
- [ ] T018 [US1] Verify CMake configure succeeds: `cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug`
- [ ] T019 [US1] Verify full build succeeds: `cmake --build build`
- [ ] T020 [US1] Verify Catch2 test discovery and execution: `ctest --test-dir build --verbose`

**Checkpoint**: Project builds from clean checkout, all dependencies fetched via FetchContent, Catch2 test passes.

---

## Phase 4: User Story 2 - Run Application (Priority: P1)

**Goal**: Running the compiled application prints "hello world" to standard output and exits with a success code.

**Independent Test**: Executing `./build/bin/iron-cube` prints "hello world" and returns exit code 0.

### Implementation for User Story 2

- [ ] T021 [US1] [US2] Implement `src/app/src/main.cpp` with `#include <iostream>` and `std::cout << "hello world" << std::endl;` returning 0 from main
- [ ] T022 [US2] Rebuild and verify: `cmake --build build && ./build/bin/iron-cube` outputs "hello world"
- [ ] T023 [US2] Verify exit code: `./build/bin/iron-cube && echo "Exit code: $?"` shows exit code 0

**Checkpoint**: Application runs, prints "hello world", and exits cleanly.

---

## Phase 5: Polish & Cross-Cutting Concerns

**Purpose**: Validation, incremental build verification, and documentation compliance.

- [ ] T024 Verify incremental build: modify `src/app/src/main.cpp`, rebuild, confirm only changed file is recompiled
- [ ] T025 [P] Validate quickstart.md commands against working build: `specs/002-build-system/quickstart.md`
- [ ] T026 [P] Verify Doxygen documentation generation: `doxygen docs/Doxyfile`
- [ ] T027 Verify compile_commands.json exists in build directory for tooling integration

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — BLOCKS all user stories
- **User Story 1 (Phase 3)**: Depends on Foundational phase completion
- **User Story 2 (Phase 4)**: Depends on Foundational phase completion and US1 build success
- **Polish (Phase 5)**: Depends on all user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Build project from source — can start after Foundational phase. No dependencies on other stories.
- **User Story 2 (P1)**: Run application — depends on US1 producing a compiled binary. Can begin implementation (main.cpp) in parallel with US1 build verification.

### Within Each User Story

- Tests MUST be written and FAIL before implementation
- CMake targets wired before source files written
- Build verification before run verification
- Story complete before moving to next priority

### Parallel Opportunities

- T003–T010: All FetchContent declarations can be added in parallel (same file, but independent blocks)
- T011–T014: CMakeLists for core, app, tests, and Doxyfile can be written in parallel (different files)
- T021: main.cpp can be written in parallel with US1 build verification (T018–T020)

---

## Parallel Example: Foundational Phase

```bash
# Launch all FetchContent declarations together (independent blocks in CMakeLists.txt):
Task: "Add FetchContent declaration for GLFW in CMakeLists.txt"
Task: "Add FetchContent declaration for Dear ImGui in CMakeLists.txt"
Task: "Add FetchContent declaration for spdlog in CMakeLists.txt"
Task: "Add FetchContent declaration for GLM in CMakeLists.txt"
Task: "Add FetchContent declaration for stb in CMakeLists.txt"
Task: "Add FetchContent declaration for tinyobjloader in CMakeLists.txt"
Task: "Add FetchContent declaration for Vulkan Headers in CMakeLists.txt"
Task: "Add FetchContent declaration for Catch2 in CMakeLists.txt"

# Launch all CMake target definitions in parallel (different files):
Task: "Write src/core/CMakeLists.txt with iron-core target"
Task: "Write src/app/CMakeLists.txt with iron-cube target"
Task: "Write tests/CMakeLists.txt with Catch2 configuration"
Task: "Write docs/Doxyfile with project configuration"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (directory structure, root CMakeLists.txt)
2. Complete Phase 2: Foundational (FetchContent deps, all CMake targets, test infra)
3. Complete Phase 3: User Story 1 (wire targets, verify build, verify tests)
4. **STOP and VALIDATE**: Project builds from clean checkout
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Verify build → MVP achieved
3. Add User Story 2 → Implement main.cpp → Verify "hello world" output
4. Polish → Incremental build check, quickstart validation, Doxygen
5. Each phase adds value without breaking previous phases

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (build verification, test infrastructure)
   - Developer B: User Story 2 (main.cpp implementation)
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies on incomplete work
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Verify Catch2 test fails before implementing build wiring
- Commit after each task or logical group
- Stop at any checkpoint to validate independently
- Existing empty stub files (`src/app/CMakeLists.txt`, `src/app/src/main.cpp`, `src/core/CMakeLists.txt`) will be populated by these tasks
