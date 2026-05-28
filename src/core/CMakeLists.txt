# Interface library for stb (header-only)
add_library(iron-stb INTERFACE)
target_include_directories(iron-stb INTERFACE ${stb_SOURCE_DIR})


# Iron core static library
add_library(
    iron-core
    STATIC
    src/core_service/servicelocator.cpp
    src/core_config/config.cpp
    src/core_platform/linux/windowglfw.cpp
    src/core_platform/linux/inputglfw.cpp
    src/core_platform/linux/platformlinux.cpp
    src/core_render/linux/vulkanrenderer.cpp
    src/core_render/linux/vulkanhelpers.cpp
    src/core_scene/scene.cpp
    src/core_resource/resourcemanager.cpp
    src/core_layer/debuglayer.cpp
    src/core_layer/scenelayer.cpp
    src/core_material/material.cpp
    src/core_material/materialservice.cpp
    src/core_shader/vulkanshader.cpp
    src/core_shader/shaderservice.cpp
    src/core_mesh/mesh.cpp
    src/core_mesh/meshservice.cpp
    src/core_ecs/ecs.cpp
    src/core_ecs/component/transformcomponent.cpp
    src/core_ecs/component/meshcomponent.cpp
    src/core_common/timer.cpp
    src/core_common/types.cpp
    src/core_common/loggerservice.cpp
    src/core_common/helpers.cpp
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
