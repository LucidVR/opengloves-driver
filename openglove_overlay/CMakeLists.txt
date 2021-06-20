cmake_minimum_required (VERSION 3.8)


# Add source to this project's executable.
add_executable (openglove_overlay "main.cpp" "main.h")

target_include_directories("openglove_overlay" PUBLIC "${OPENVR_INCLUDE_DIR}")
target_link_libraries("openglove_overlay" PUBLIC "${OPENVR_LIB}")