# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(device_drivers STATIC knuckle_device_driver.h knuckle_device_driver.cpp device_driver.h)

find_package(OpenVR REQUIRED)

target_link_libraries(device_drivers PUBLIC OpenVR::OpenVR opengloves_interface-includes)
target_link_libraries(device_drivers PRIVATE driver-includes driver_utils hand_tracking device_pose)