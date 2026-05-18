# Architectural Cleanup

Based on your feedback, I will consolidate all core logic components (Core Manager, Utilities, Contracts) from the feature-specific directory (`specs/001-gfx-renderer-viewer/`) into the top-level `src/` directory to maintain a clean, scalable architecture.

This involves:
1.  Moving header and source files for `CentralCoreManager`, `DebugSettings`, `PluginManager` to `src/core/`.
2.  Moving utility code (`serializer.cpp`, etc.) to `src/util/`.
3.  Updating all relevant contract definitions to use the relative path `src/...` instead of `specs/001-gfx-renderer-viewer/src/...`.

I will update the file contents for the affected files accordingly and then run the scaffold commands again on the corrected paths, assuming the directories now exist in a common `src/` location.
