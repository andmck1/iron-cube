## Clarifications

### Session 2026-05-18 - Architectural Contracts

*   **Q: Renderer API Contract Definition → A:** The configuration API for plugins MUST be exposed via a standard, versioned C++/Python interface (`IRenderConfigurator`) that all external modules must implement and register.
*   **Q: Data Ownership and State Management → A:** All debug settings MUST be managed by the Central Core Manager to serve as the single source of truth, providing controlled read/write APIs for all interacting components.
*   **Q: Cross-Module Communication Protocol → A:** The primary communication protocol between the renderer/app shell and any extension module MUST utilize a standardized RPC/Message Bus system (e.g., gRPC) with defined message schemas to ensure reliable data exchange.