# Interface library for stb (header-only)
add_library(iron-stb INTERFACE)
target_include_directories(iron-stb INTERFACE ${stb_SOURCE_DIR})


# Iron core static library
add_library(
    iron-core
    STATIC
    src/service/servicelocator.cpp
    src/config/config.cpp
    src/platform/linux/windowglfw.cpp
    src/platform/linux/inputglfw.cpp
    src/platform/linux/platformlinux.cpp
    src/render/linux/vulkanrenderer.cpp
    src/render/linux/vulkanhelpers.cpp
    src/scene/scene.cpp
    src/resource/resourcemanager.cpp
    src/layer/debuglayer.cpp
    src/layer/scenelayer.cpp
    src/material/material.cpp
    src/material/materialsservice.cpp
    src/shader/shader.cpp
    src/mesh/mesh.cpp
    src/mesh/meshesservice.cpp
    src/ecs/ecs.cpp
    src/ecs/component/transformcomponent.cpp
    src/ecs/component/meshcomponent.cpp
    src/common/timer.cpp
    src/common/types.cpp
    src/common/loggerservice.cpp
    src/common/helpers.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(iron-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(iron-core PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(iron-core PUBLIC ${imgui_SOURCE_DIR}/backends)

target_link_libraries(
    iron-core
    PUBLIC
        glfw
        spdlog::spdlog
        glm::glm
        iron-stb
        tinyobjloader
        Vulkan::Vulkan
        yaml-cpp
)
