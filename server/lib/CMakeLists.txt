add_library(server_lib-includes INTERFACE)

target_include_directories(server_lib-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})