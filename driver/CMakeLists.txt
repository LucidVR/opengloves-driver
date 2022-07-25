cmake_minimum_required(VERSION 3.7.1)
set(CMAKE_CXX_STANDARD 20)

# solution
project("opengloves_openvr_driver")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# driver setup
set(DRIVER_NAME "driver_opengloves")
set(OPENGLOVES_PROJECT "driver_${DRIVER_NAME}")

# get commit hash
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)


#
## openvr
#set(OPENVR_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/openvr/headers")
#find_library(OPENVR_LIB openvr_api HINTS "${CMAKE_CURRENT_SOURCE_DIR}/libraries/openvr/lib/${PLATFORM_NAME}${PROCESSOR_ARCH}/" NO_DEFAULT_PATH)
#target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${OPENVR_INCLUDE_DIR}")
#target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC "${OPENVR_LIB}")
#
## tinygltf
#set(TINYGLTF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries/tinygltf")
#target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${TINYGLTF_INCLUDE_DIR}")
#
## opengloves
#target_include_directories(${OPENGLOVES_PROJECT} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#target_compile_definitions(${OPENGLOVES_PROJECT} PRIVATE
#        "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
#
#
#if (WIN32)
#    target_link_libraries(${OPENGLOVES_PROJECT} PUBLIC setupapi wsock32 ws2_32 bthprops)
#endif ()
#
## Copy driver assets to output folder
#add_custom_command(
#        TARGET ${OPENGLOVES_PROJECT}
#        PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/${DRIVER_NAME}
#        $<TARGET_FILE_DIR:${OPENGLOVES_PROJECT}>/${DRIVER_NAME}
#)
#
## Copy dll to output folder
#add_custom_command(
#        TARGET ${OPENGLOVES_PROJECT}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        $<TARGET_FILE:${OPENGLOVES_PROJECT}>
#        $<TARGET_FILE_DIR:${OPENGLOVES_PROJECT}>/${DRIVER_NAME}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}/$<TARGET_FILE_NAME:${OPENGLOVES_PROJECT}>
#)