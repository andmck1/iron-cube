# Compile Slang shaders to SPIR-V
set(SHADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shader)
set(SPIRV_BASE_DIR ${CMAKE_BINARY_DIR}/shader)

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
        list(
            APPEND SHADER_DEPENDENCIES
            ${SHADER_BASE_DIR}/buffer/ubuffer.h
        )
    endif()

    add_custom_command(
        OUTPUT ${SPIRV_FILE}
        COMMAND
            ${SLANGC_EXECUTABLE} -target spirv -entry main -stage ${STAGE} -I
            ${CMAKE_CURRENT_SOURCE_DIR}/src -o ${SPIRV_FILE} ${SHADER_FILE}
        DEPENDS ${SHADER_DEPENDENCIES}
        COMMENT "Compiling ${SHADER_FILE} -> ${SPIRV_FILE}"
    )
    list(APPEND ALL_SPIRV_FILES ${SPIRV_FILE})
endforeach()

if(ALL_SPIRV_FILES)
    add_custom_target(compile_shaders ALL DEPENDS ${ALL_SPIRV_FILES})
endif()

# Iron Cube executable
add_executable(iron-cube src/main.cpp src/application/application.cpp)

target_link_libraries(iron-cube PRIVATE iron-core)

add_dependencies(iron-cube compile_shaders)
