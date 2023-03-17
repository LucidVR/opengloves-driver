# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(server_services_input STATIC input_force_feedback_named_pipe.h input_force_feedback_named_pipe.cpp)

target_include_directories(server_services_input PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(server_services_input PUBLIC opengloves_interface-includes server_lib-includes)
