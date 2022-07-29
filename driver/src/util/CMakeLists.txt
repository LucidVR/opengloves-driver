add_library(driver_utils STATIC driver_log.h driver_log.cpp)

target_include_directories(driver_utils PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")