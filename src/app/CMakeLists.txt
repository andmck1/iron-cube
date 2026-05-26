# Iron Cube executable
add_executable(iron-cube src/main.cpp src/application/application.cpp)

target_link_libraries(iron-cube PRIVATE
  iron-core
)

add_dependencies(iron-cube compile_shaders)
