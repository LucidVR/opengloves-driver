# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_subdirectory(lucidgloves)

add_library(opengloves_devices-includes INTERFACE)
target_include_directories(opengloves_devices-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})