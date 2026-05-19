# Interface library for stb (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# Iron core static library
add_library(iron-core STATIC
    src/core_stub.cpp
    src/service/ServiceLocator.cpp
    src/platform/linux/WindowGLFW.cpp
    src/platform/linux/InputGLFW.cpp
    src/platform/linux/PlatformLinux.cpp
    src/rendering/linux/RendererGLFW.cpp
)

target_include_directories(iron-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(iron-core PUBLIC
  glfw
  spdlog::spdlog
  glm::glm
  stb
  tinyobjloader
  Vulkan::Headers
)
