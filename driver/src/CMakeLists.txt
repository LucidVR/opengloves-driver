add_library(driver-includes INTERFACE)

target_include_directories(driver-includes INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(util)
add_subdirectory(device_configuration)
add_subdirectory(device_providers)
add_subdirectory(device_drivers)
add_subdirectory(hand_tracking)
add_subdirectory(device_pose)

add_library(driver_factory STATIC driver_factory.cpp)
target_link_libraries(driver_factory PRIVATE openvr-includes driver-includes driver_utils device_providers)