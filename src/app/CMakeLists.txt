# Iron Cube executable
add_executable(iron-cube src/main.cpp src/Application.cpp)

target_link_libraries(iron-cube PRIVATE
  iron-core
)
