# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_subdirectory(communication)
add_subdirectory(services)
add_subdirectory(device)

add_library(server-includes INTERFACE)
target_include_directories(server-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(opengloves_server STATIC opengloves_server.cpp)

target_link_libraries(opengloves_server PUBLIC opengloves_interface-includes)
target_link_libraries(opengloves_server PRIVATE communication_managers device_lucidgloves)