# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(opengloves_interface-includes INTERFACE)

target_include_directories(opengloves_interface-includes INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(opengloves_interface-includes INTERFACE communication_managers opengloves_devices-includes)

add_subdirectory(lib)
add_subdirectory(src)