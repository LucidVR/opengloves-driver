# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(device_lucidgloves STATIC
        lucidgloves_device.h
        lucidgloves_device.cpp
        discovery/lucidgloves_fw_discovery.h
        discovery/lucidgloves_fw_discovery.cpp
        discovery/lucidgloves_named_pipe_discovery.h
        discovery/lucidgloves_named_pipe_discovery.cpp
        )

target_include_directories(device_lucidgloves PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

target_link_libraries(device_lucidgloves PUBLIC opengloves_interface-includes server-includes)
target_link_libraries(device_lucidgloves PRIVATE server_services_output server_services_input communication_probers)