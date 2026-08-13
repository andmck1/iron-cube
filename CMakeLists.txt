# AI help here

cmake_minimum_required(VERSION 4.0.2)
project(IronCube LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-4 -O0")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-Wl,-debug")
endif()

set(FETCHCONTENT_QUIET ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/syoyo/tinyobjloader.git
    GIT_TAG HEAD
)
FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
    meshopt
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG HEAD
)
FetchContent_MakeAvailable(meshopt)

# Use system Vulkan
find_package(Vulkan REQUIRED)

# Use system slangc for shader compilation
find_program(SLANGC_EXECUTABLE slangc REQUIRED)

FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.0
)
FetchContent_MakeAvailable(cli11)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG origin/master
)
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    ktx
    GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
    GIT_TAG origin/main
)
FetchContent_MakeAvailable(ktx)

if(WIN32)
    find_library(
        NVML_LIBRARY
        NAMES nvml
        PATHS "$ENV{CUDA_PATH}/lib/x64"
        REQUIRED
    )
    find_path(
        NVML_INCLUDE_DIR
        NAMES nvml.h
        PATHS "$ENV{CUDA_PATH}/include"
        NO_DEFAULT_PATH
        REQUIRED
    )
else()
    find_library(NVML_LIBRARY NAMES nvidia-ml REQUIRED)
    find_path(
        NVML_INCLUDE_DIR
        NAMES nvml.h
        PATHS /opt/cuda/include /opt/cuda/targets/x86_64-linux/include
        NO_DEFAULT_PATH
        REQUIRED
    )
endif()

message(STATUS "NVML library: ${NVML_LIBRARY}")
message(STATUS "NVML include dir: ${NVML_INCLUDE_DIR}")

add_library(NVML::NVML UNKNOWN IMPORTED)
set_target_properties(
    NVML::NVML
    PROPERTIES
        IMPORTED_LOCATION "${NVML_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${NVML_INCLUDE_DIR}"
)

add_subdirectory(src/core)
add_subdirectory(src/app)
