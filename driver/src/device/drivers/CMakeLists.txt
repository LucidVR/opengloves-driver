add_library(device_drivers STATIC knuckle_device_driver.h knuckle_device_driver.cpp)

find_package(OpenVR REQUIRED)

target_link_libraries(device_drivers PUBLIC OpenVR::OpenVR device-includes opengloves_server-includes)
target_link_libraries(device_drivers PRIVATE driver_utils driver_utils-includes hand_tracking hand_tracking-includes)