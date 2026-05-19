# Build System

## Dependencies
This project requires the following external libraries, all managed via CMake FetchContent for reproducibility:

- GLFW (Window management/Input)
- Dear ImGui (GUI tool/debug overlay)
- spdlog (Logging)
- GLM (Math library)
- stb (Image/font utilities)
- tinyobjloader (Model loading)
- Vulkan Headers (Graphics API context)
- Catch2 (Testing framework)
- Doxygen (Documentation generation - Build Tool)

## Configuration Note
All dependencies must be included in the root `CMakeLists.txt` using CMake FetchContent to ensure a clean, isolated build environment regardless of system package managers.

## Status
- [x] Root CMakeLists.txt with FetchContent dependencies
- [x] Core library (iron-core) with stub source
- [x] Application target (iron-cube) with minimal main
- [x] Build verification tests
- [x] Clean build and test execution

