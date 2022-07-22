cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# platform
set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROCESSOR_ARCH "64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PROCESSOR_ARCH "32")
endif ()

if (WIN32)
    set(PLATFORM_NAME "win")
elseif (UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
elseif (APPLE)
    set(PLATFORM_NAME "osx")
endif ()

project(opengloves)

add_subdirectory(server)
add_subdirectory(driver)