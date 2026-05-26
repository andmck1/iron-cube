# Interface library for stb (header-only)
add_library(iron-stb INTERFACE)
target_include_directories(iron-stb INTERFACE ${stb_SOURCE_DIR})

# Compile Slang shaders to SPIR-V
set(SHADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/slang)
set(SPIRV_BASE_DIR ${CMAKE_BINARY_DIR}/shader/slang)

file(MAKE_DIRECTORY ${SPIRV_BASE_DIR})

file(GLOB_RECURSE ALL_SHADERS "${SHADER_BASE_DIR}/*.slang")

set(ALL_SPIRV_FILES "")

foreach(SHADER_FILE ${ALL_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    
    if(SHADER_NAME MATCHES ".vert.slang")
        set(STAGE "vertex")
    elseif(SHADER_NAME MATCHES ".frag.slang")
        set(STAGE "fragment")
    else()
        continue()
    endif()

    file(RELATIVE_PATH REL_PATH ${SHADER_BASE_DIR} ${SHADER_FILE})
    string(REPLACE ".slang" ".spv" SPIRV_FILE_REL_PATH ${REL_PATH})
    set(SPIRV_FILE "${SPIRV_BASE_DIR}/${SPIRV_FILE_REL_PATH}")

    set(SHADER_DEPENDENCIES ${SHADER_FILE})
    if(STAGE STREQUAL "vertex")
        list(APPEND SHADER_DEPENDENCIES ${CMAKE_CURRENT_SOURCE_DIR}/src/buffer/ubuffer.h)
    endif()

    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND
            ${SLANGC_EXECUTABLE} -target spirv -entry main -stage ${STAGE}
            -I ${CMAKE_CURRENT_SOURCE_DIR}/src
            -o ${SPIRV_FILE} ${SHADER_FILE}
        DEPENDS ${SHADER_DEPENDENCIES}
        COMMENT "Compiling ${SHADER_FILE} -> ${SPIRV_FILE}"
    )
    list(APPEND ALL_SPIRV_FILES ${SPIRV_FILE})
endforeach()

if(ALL_SPIRV_FILES)
    add_custom_target(
        compile_shaders
        ALL
        DEPENDS ${ALL_SPIRV_FILES}
    )
endif()

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
