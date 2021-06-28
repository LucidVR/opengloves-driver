cmake_minimum_required (VERSION 3.8)


# Add source to this project's executable.
add_executable (openglove_overlay WIN32 "main.cpp" "main.h")

target_include_directories("openglove_overlay" PUBLIC "${OPENVR_INCLUDE_DIR}")
target_link_libraries("openglove_overlay" PUBLIC "${OPENVR_LIB}")

add_custom_command(TARGET openglove_overlay POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/libraries/openvr/bin/win64/openvr_api.dll"
    $<TARGET_FILE_DIR:openglove_overlay>)