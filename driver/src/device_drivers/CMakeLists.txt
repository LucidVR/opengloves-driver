add_library(device_drivers STATIC knuckle_device_driver.h knuckle_device_driver.cpp)

target_link_libraries(device_drivers PRIVATE openvr-includes driver-includes)
target_include_directories(device_drivers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")