# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

add_library(hand_tracking STATIC hand_tracking.h hand_tracking.cpp anim_loader.h anim_loader.cpp)
target_include_directories(hand_tracking PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${TINYGLTF_INCLUDE_DIRS})

target_link_libraries(hand_tracking PUBLIC opengloves_interface-includes)
target_link_libraries(hand_tracking PRIVATE driver-includes driver_utils)