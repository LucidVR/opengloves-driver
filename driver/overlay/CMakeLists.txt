# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(restclient-cpp CONFIG REQUIRED)
find_package(OpenVR REQUIRED)

if(WIN32)
add_executable(opengloves_overlay WIN32 main.cpp)
else()
add_executable(opengloves_overlay main.cpp)
endif()

target_link_libraries(opengloves_overlay PRIVATE OpenVR::OpenVR CURL::libcurl restclient-cpp nlohmann_json::nlohmann_json)