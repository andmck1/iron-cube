# Compile Slang shaders to SPIR-V
set(SHADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shader)
set(SPIRV_BASE_DIR ${CMAKE_BINARY_DIR}/shader)
set(SHARED_CORE_BUFFERTYPES_HEADER
    ${CMAKE_SOURCE_DIR}/src/core/src/core_buffertypes/buffertypes.h
)
set(SHARED_APP_BUFFERTYPES_HEADER ${SHADER_BASE_DIR}/shared/buffertypes.h)

file(MAKE_DIRECTORY ${SPIRV_BASE_DIR})
file(GLOB_RECURSE ALL_SHADERS "${SHADER_BASE_DIR}/*.slang")
set(ALL_SPIRV_FILES "")

foreach(SHADER_FILE ${ALL_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)

    if(SHADER_NAME MATCHES ".vert.slang")
        set(STAGE "vertex")
    elseif(SHADER_NAME MATCHES ".frag.slang")
        set(STAGE "fragment")
    elseif(SHADER_NAME MATCHES ".mesh.slang")
        set(STAGE "mesh")
    elseif(SHADER_NAME MATCHES ".task.slang")
        set(STAGE "task")
    else()
        continue()
    endif()

    file(RELATIVE_PATH REL_PATH ${SHADER_BASE_DIR} ${SHADER_FILE})
    string(REPLACE ".slang" ".spv" SPIRV_FILE_REL_PATH ${REL_PATH})
    set(SPIRV_FILE "${SPIRV_BASE_DIR}/${SPIRV_FILE_REL_PATH}")

    set(SHADER_DEPENDENCIES ${SHADER_FILE})
    list(APPEND SHADER_DEPENDENCIES ${SHARED_APP_BUFFERTYPES_HEADER})
    list(APPEND SHADER_DEPENDENCIES ${SHARED_CORE_BUFFERTYPES_HEADER})

    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND
            ${SLANGC_EXECUTABLE} -target spirv -entry main -stage ${STAGE} -I
            ${CMAKE_CURRENT_SOURCE_DIR}/src -I ${SHADER_BASE_DIR} -I
            ${CMAKE_SOURCE_DIR}/src/core/src -o ${SPIRV_FILE} ${SHADER_FILE}
        DEPENDS ${SHADER_DEPENDENCIES}
        COMMENT "Compiling ${SHADER_FILE} -> ${SPIRV_FILE}"
    )
    list(APPEND ALL_SPIRV_FILES ${SPIRV_FILE})
endforeach()

if(ALL_SPIRV_FILES)
    add_custom_target(compile_shaders ALL DEPENDS ${ALL_SPIRV_FILES})
endif()

# Iron Cube executable
add_executable(
    iron-cube
    src/main.cpp
    src/application/application.cpp
    src/shaderpipeline/standard.cpp
    src/shaderpipeline/instancedMesh.cpp
)

target_link_libraries(iron-cube PRIVATE iron-core)

target_include_directories(iron-cube PRIVATE src src/shader)

add_dependencies(iron-cube compile_shaders)
