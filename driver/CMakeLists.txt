cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

# solution
project("opengloves_openvr_driver")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# driver setup
add_subdirectory(src)

set(DRIVER_NAME "opengloves")
set(OPENGLOVES_PROJECT "driver_${DRIVER_NAME}")

add_library("${OPENGLOVES_PROJECT}" SHARED src/driver_factory.cpp)

#openvr
find_package(OpenVR REQUIRED)
target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC OpenVR::OpenVR)

#tinygltf
set(TINYGLTF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/tinygltf")
target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${TINYGLTF_INCLUDE_DIR}")

## opengloves
target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC opengloves_server-includes device_providers)
target_link_libraries(${OPENGLOVES_PROJECT} PRIVATE opengloves_server)


# Copy driver assets to output folder
add_custom_command(
        TARGET ${OPENGLOVES_PROJECT}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${DRIVER_NAME}
        $<TARGET_FILE_DIR:${OPENGLOVES_PROJECT}>/${DRIVER_NAME}
)

# Copy dll to output folder
add_custom_command(
        TARGET ${OPENGLOVES_PROJECT}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${OPENGLOVES_PROJECT}>
        $<TARGET_FILE_DIR:${OPENGLOVES_PROJECT}>/${DRIVER_NAME}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:${OPENGLOVES_PROJECT}>
)