find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenVR REQUIRED)

add_executable(opengloves_overlay main.cpp)

target_link_libraries(opengloves_overlay PRIVATE opengloves_protos OpenVR::OpenVR)

add_custom_command(
        TARGET opengloves_overlay
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        $<TARGET_FILE_DIR:opengloves_overlay>
        $<TARGET_FILE_DIR:${OPENGLOVES_PROJECT}>/${DRIVER_NAME}/bin/${PLATFORM_NAME}${PROCESSOR_ARCH}
)