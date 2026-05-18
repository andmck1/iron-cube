# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.18)

project(VulkanRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set include directories for all modules to find headers in src/
include_directories(src)

# Define the core executables and libraries
add_executable(${PROJECT_NAME} src/main_renderer_loop.cpp)

# Core Libraries (These contain shared functionality like Manager, Utils, Contracts)
add_library(Core STATIC src/core/central_manager.cpp src/core/debug_settings.cpp src/core/plugin_manager.cpp)
target_include_directories(Core PUBLIC src/) # Expose core headers to all modules

# Utilities Library (Logger, Serializer)
add_library(Util STATIC src/util/logger.cpp src/util/serializer.cpp)
target_include_directories(Util PUBLIC src/)

# Contracts and Plugin API (Should be minimal or header-only for simplicity in V1)
add_library(Contracts INTERFACE src/contracts/interface_renderer_configurator.h)

# Renderer Backend Library 
add_library(Renderer STATIC src/renderer/vulkan_context.cpp src/renderer/basic_pass.cpp)
target_link_libraries(Renderer PRIVATE Core Contracts Util) # Linking to dependencies

# Dependencies (Placeholder for Vulkan, ImGui, etc.)
find_package(Vulkan REQUIRED) 
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})

# Link all components together in the final executable
target_link_libraries(${PROJECT_NAME} PRIVATE Core Util Contracts Renderer)