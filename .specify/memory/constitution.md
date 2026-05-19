<!--
Sync Impact Report:
  Version change: none → 1.0.0 (initial ratification)
  Principles added:
    - I. Test-Driven Development
    - II. Documentation First
    - III. Layered & Component-Based Architecture
    - IV. Data-Oriented Design
    - V. Service-Locator Pattern
    - VI. Incremental Feature Development
  Sections added:
    - Technical Constraints
    - Development Workflow
  Templates requiring updates:
    - .specify/templates/plan-template.md: ✅ no changes needed (generic Constitution Check section)
    - .specify/templates/spec-template.md: ✅ no changes needed (already aligns with TDD and documentation-first)
    - .specify/templates/tasks-template.md: ✅ no changes needed (already includes TDD task phases)
  Follow-up TODOs: none
-->

# Iron Cube Constitution

## Core Principles

### I. Test-Driven Development

TDD is mandatory for all new functionality. Tests MUST be written before implementation code. The Red-Green-Refactor cycle is strictly enforced:

- Tests are written first and MUST fail before any implementation begins
- Implementation is written only to make the failing test pass
- Refactoring follows only after tests pass
- No code is merged without corresponding test coverage

**Rationale**: Writing tests first clarifies requirements, prevents over-engineering, and ensures every line of code serves a verified purpose.

### II. Documentation First

Documentation drives development, not the reverse. Feature specifications, API contracts, and design documents MUST exist before implementation begins:

- Feature specs define user stories, acceptance criteria, and success metrics
- API contracts are written before service implementations
- Architectural decisions are documented before encoding them in code
- Documentation is treated as a first-class deliverable, not an afterthought

**Rationale**: Documentation-first prevents scope creep, aligns stakeholders, and provides the specification against which tests are written.

### III. Layered & Component-Based Architecture

The codebase follows a dual architecture model:

- **High-level**: Layered architecture providing clear abstraction boundaries. Each layer depends only on layers below it.
- **Low-level**: Component-based architecture where composition is prioritized over inheritance. Components are small, focused, and combinable.

Inheritance chains are discouraged. Complex behavior emerges from composing simple, well-defined components rather than deep class hierarchies.

**Rationale**: Layered abstraction manages complexity at scale; composition over inheritance keeps low-level code flexible and reusable.

### IV. Data-Oriented Design

Scene data is stored for cache efficiency and data locality, not encapsulated within classes:

- Data structures are organized for optimal memory access patterns
- Classes do not hold scene state — data resides in structured, contiguous storage
- Components operate on data collections, not on individual object instances
- Data layout decisions consider CPU cache lines and SIMD-friendly access

**Rationale**: Data-oriented design maximizes hardware utilization and avoids the performance penalty of pointer-chasing through object-oriented hierarchies.

### V. Service-Locator Pattern

Services are derived from common interfaces and consumed by clients via a service locator:

- Services expose well-defined interfaces, not concrete implementations
- Clients request services from the locator rather than constructing dependencies directly
- The locator manages service lifecycle and provides single access points
- New services integrate by registering against an interface — no client code changes required

**Rationale**: Decouples consumers from providers, enables swapping implementations, and simplifies test mocking.

### VI. Incremental Feature Development

Features are delivered in small, progressive increments:

- Each increment MUST be independently testable and demonstrable
- Work is broken into the smallest viable units that deliver value
- No feature is so large that it cannot be completed and tested within a single development cycle
- Progress is measured by working, tested increments — not by lines of code

**Rationale**: Small increments reduce risk, enable early feedback, and ensure constant forward progress.

## Technical Constraints

**Language**: C++20
**Build System**: CMake (minimum version 4.0.2)
**Graphics API**: Vulkan
**Platform**: Linux (primary)

Compile commands are exported (`CMAKE_EXPORT_COMPILE_COMMANDS`) for tooling integration. Debug builds include DWARF-4 debug symbols with no optimization.

## Development Workflow

**Feature Branching**: Each feature develops on a dedicated branch named `[###-feature-name]`.

**Constitution Check**: Before implementation planning, verify the feature plan passes all Constitution Check gates. Re-check after design finalization.

**Quality Gates**:
- Tests MUST pass before merge
- Documentation MUST be updated alongside code changes
- Code MUST adhere to architectural principles (layered, component-based, data-oriented)
- Service interfaces MUST be stable before client consumption

**Review Process**: All changes require verification that:
1. Tests were written before implementation (TDD compliance)
2. Documentation was updated (Documentation First compliance)
3. No inheritance was introduced where composition suffices
4. Data structures favor locality over encapsulation
5. Services use interface-based contracts via the locator

## Governance

This Constitution supersedes all other development practices and conventions for this project.

**Amendment Process**:
- Amendments require written documentation of the change
- Each amendment must include a rationale and migration plan if existing code is affected
- Backward-incompatible principle changes require review of all active features

**Compliance**:
- All PRs and code reviews MUST verify compliance with applicable principles
- Unjustified complexity violations MUST be documented with reasoning
- Use `AGENTS.md` and project README for runtime development guidance

**Versioning**:
- MAJOR: Backward-incompatible governance changes, principle removals, or redefinitions
- MINOR: New principle/section added or materially expanded guidance
- PATCH: Clarifications, wording fixes, non-semantic refinements

**Version**: 1.0.0 | **Ratified**: 2026-05-19 | **Last Amended**: 2026-05-19
