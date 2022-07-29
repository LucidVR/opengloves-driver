add_library(hand_tracking STATIC hand_tracking.h hand_tracking.cpp anim_loader.h anim_loader.cpp)

target_include_directories(hand_tracking PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")