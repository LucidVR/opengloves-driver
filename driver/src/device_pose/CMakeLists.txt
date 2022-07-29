add_library(device_pose STATIC device_pose.h device_pose.cpp pose_calibration.h pose_calibration.cpp)

target_include_directories(device_pose PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")