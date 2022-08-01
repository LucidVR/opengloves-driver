add_library(opengloves_server-includes INTERFACE)

target_include_directories(opengloves_server-includes INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(opengloves_server-includes INTERFACE communication_managers device_discoverers communication-includes opengloves_devices-includes)

add_subdirectory(src)