find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

add_library(hand_tracking-includes INTERFACE)
target_include_directories(hand_tracking-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(hand_tracking STATIC hand_tracking.h hand_tracking.cpp anim_loader.h anim_loader.cpp)

target_include_directories(hand_tracking PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${TINYGLTF_INCLUDE_DIRS})

target_link_libraries(hand_tracking PUBLIC opengloves_server-includes)

target_link_libraries(hand_tracking PRIVATE driver_utils driver_utils-includes)