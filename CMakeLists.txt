cmake_minimum_required(VERSION "3.7.1")

# Solution
project("openglove")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Deps
set(OPENVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/openvr/headers")
set(ASIO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/asio/asio/include")

set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})

include(cmake/steamvrPaths.cmake)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROCESSOR_ARCH "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PROCESSOR_ARCH "32")
endif()

if(WIN32)
    set(PLATFORM_NAME "win")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
elseif(APPLE)
    set(PLATFORM_NAME "osx")
endif()

find_library(OPENVR_LIB openvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/libraries/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/" NO_DEFAULT_PATH )

set(DRIVER_NAME "openglove")
set(OPENGLOVE_PROJECT "driver_${DRIVER_NAME}")

file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library("${OPENGLOVE_PROJECT}" SHARED "${HEADERS}" "${SOURCES}")

target_include_directories("${OPENGLOVE_PROJECT}" PUBLIC "${OPENVR_INCLUDE_DIR}" "${ASIO_INCLUDE_DIR}")

target_include_directories("${OPENGLOVE_PROJECT}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries("${OPENGLOVE_PROJECT}" PUBLIC "${OPENVR_LIB}")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCES})
set_property(TARGET "${OPENGLOVE_PROJECT}" PROPERTY CXX_STANDARD 17)

# Copy driver assets to output folder
add_custom_command(
        TARGET ${OPENGLOVE_PROJECT}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${DRIVER_NAME}
        $<TARGET_FILE_DIR:${OPENGLOVE_PROJECT}>/${DRIVER_NAME}
)

# Copy dll to output folder
add_custom_command(
        TARGET ${OPENGLOVE_PROJECT}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${OPENGLOVE_PROJECT}>
        $<TARGET_FILE_DIR:${OPENGLOVE_PROJECT}>/${DRIVER_NAME}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:${OPENGLOVE_PROJECT}>
)

add_custom_command(
        TARGET ${OPENGLOVE_PROJECT}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:${OPENGLOVE_PROJECT}>/${DRIVER_NAME}
        ${STEAMVR_DRIVER_DIR}/${DRIVER_NAME}
)