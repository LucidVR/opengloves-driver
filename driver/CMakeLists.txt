cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

project("opengloves_openvr_driver")

set(DRIVER_NAME "opengloves")
set(OPENGLOVES_PROJECT "driver_${DRIVER_NAME}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${DRIVER_NAME}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH})

message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_library(${OPENGLOVES_PROJECT} SHARED src/driver_factory.cpp)
set_target_properties(${OPENGLOVES_PROJECT} PROPERTIES PREFIX "") # on linux cmake prefixes the driver static library with lib
set_target_properties(${OPENGLOVES_PROJECT} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) # for linux

# driver setup
add_subdirectory(overlay)
add_subdirectory(src)

# openvr
find_package(OpenVR REQUIRED)
target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC OpenVR::OpenVR)

## opengloves
target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC opengloves_interface-includes)
target_link_libraries(${OPENGLOVES_PROJECT} PRIVATE opengloves_server device_providers device_drivers)

# Copy driver assets to output folder
add_custom_command(
        TARGET ${OPENGLOVES_PROJECT}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${DRIVER_NAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${DRIVER_NAME}
)