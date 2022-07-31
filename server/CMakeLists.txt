add_library(opengloves_server-includes INTERFACE)

target_include_directories(opengloves_server-includes INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)