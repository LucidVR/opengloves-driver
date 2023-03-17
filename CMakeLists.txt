# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(opengloves)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_compile_definitions(_USE_MATH_DEFINES)

# platform
if(NOT PLATFORM)
    if(CMAKE_SIZEOF_VOID_P MATCHES 8)
        set(PROCESSOR_ARCH 64)
    else()
        set(PROCESSOR_ARCH 32)
    endif()
endif()
message(STATUS "Compilation set for ${PROCESSOR_ARCH}bits architectures.")


if (WIN32)
    set(PLATFORM_NAME "win")

    add_definitions(-D_WIN32_WINNT=0x0601)
elseif (UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
elseif (APPLE)
    set(PLATFORM_NAME "osx")
endif ()

add_subdirectory(server)
add_subdirectory(driver)

