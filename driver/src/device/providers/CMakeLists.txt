# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(device_providers STATIC physical_device_provider.h physical_device_provider.cpp)

find_package(OpenVR REQUIRED)

target_link_libraries(device_providers PUBLIC OpenVR::OpenVR opengloves_interface-includes)
target_link_libraries(device_providers PRIVATE driver-includes driver_utils device_configuration device_drivers driver_services)