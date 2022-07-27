add_subdirectory(discovery)

add_library(devices STATIC opengloves_device.cpp)
target_include_directories(devices PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(devices PRIVATE opengloves_server-includes communication_managers communication-includes)