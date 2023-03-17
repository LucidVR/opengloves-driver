# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(driver-includes INTERFACE)

target_include_directories(driver-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(services)
add_subdirectory(hand_tracking)
add_subdirectory(util)
add_subdirectory(device)
