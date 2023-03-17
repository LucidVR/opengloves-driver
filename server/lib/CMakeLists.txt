# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(server_lib-includes INTERFACE)

target_include_directories(server_lib-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(server_lib STATIC win/win_util.cpp)