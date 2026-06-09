# Interface library for stb (header-only)
add_library(iron-stb INTERFACE)
target_include_directories(iron-stb INTERFACE ${stb_SOURCE_DIR})

# Iron core static library
add_library(
    iron-core
    STATIC
    src/core_common/helpers.cpp
    src/core_common/timer.cpp
    src/core_common/types.cpp
    src/core_common/logger.cpp
    src/core_config/config.cpp
    src/core_ecs/component/mesh.cpp
    src/core_ecs/component/transform.cpp
    src/core_ecs/ecs.cpp
    src/core_layer/debug.cpp
    src/core_layer/debuglayerdata.cpp
    src/core_layer/scene.cpp
    src/core_mesh/mesh.cpp
    src/core_mesh/meshmanager.cpp
    src/core_platform/linux/inputglfw.cpp
    src/core_platform/linux/platform.cpp
    src/core_platform/linux/windowglfw.cpp
    src/core_render/vulkan/helpers.cpp
    src/core_render/vulkan/renderer.cpp
    src/core_render/vulkan/shader.cpp
    src/core_render/vulkan/shaderpipelinemanager.cpp
    src/core_render/vulkan/shaderpipeline.cpp
    src/core_render/vulkan/shadermanager.cpp
    src/core_resource/resourcemanager.cpp
    src/core_scene/scene.cpp
    src/core_service/servicelocator.cpp
    src/core_buffertypes/buffertypes.cpp
    src/core_actor/camera.cpp
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
        meshopt
        Vulkan::Vulkan
        yaml-cpp
)
