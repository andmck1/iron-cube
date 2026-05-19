add_executable(test_window test_window.cpp)
target_link_libraries(test_window PRIVATE Catch2::Catch2WithMain iron-core)
catch_discover_tests(test_window)

add_executable(test_platform test_platform.cpp)
target_link_libraries(test_platform PRIVATE Catch2::Catch2WithMain iron-core)
catch_discover_tests(test_platform)

add_executable(test_input test_input.cpp)
target_link_libraries(test_input PRIVATE Catch2::Catch2WithMain iron-core)
catch_discover_tests(test_input)

add_executable(test_renderer test_renderer.cpp)
target_link_libraries(test_renderer PRIVATE Catch2::Catch2WithMain iron-core)
catch_discover_tests(test_renderer)
