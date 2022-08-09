add_library(device_drivers STATIC knuckle_device_driver.h knuckle_device_driver.cpp device_driver.h device_driver.cpp)

find_package(OpenVR REQUIRED)

target_link_libraries(device_drivers PUBLIC device-includes opengloves_server-includes OpenVR::OpenVR)
target_link_libraries(device_drivers PRIVATE driver_utils driver_utils-includes)