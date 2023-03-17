# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

find_package(OpenVR REQUIRED)

add_library(driver_utils STATIC driver_log.h driver_log.cpp file_path.h file_path_win.cpp driver_math.h driver_math.cpp win_util.h win_util.cpp)
target_include_directories(driver_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(driver_utils PRIVATE OpenVR::OpenVR)