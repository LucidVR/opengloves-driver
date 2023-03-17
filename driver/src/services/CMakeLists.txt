# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

find_package(OpenVR REQUIRED)
find_package(Crow CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


add_library(driver_services STATIC driver_internal.h driver_internal.cpp webserver_logging.h webserver_logging.cpp driver_external.h driver_external.cpp)

target_include_directories(driver_services PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(driver_services PUBLIC OpenVR::OpenVR)
target_link_libraries(driver_services PRIVATE driver-includes driver_utils Crow::Crow nlohmann_json::nlohmann_json opengloves_interface-includes)