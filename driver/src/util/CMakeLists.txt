find_package(OpenVR REQUIRED)

add_library(driver_utils-includes INTERFACE)
target_include_directories(driver_utils-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(driver_utils STATIC driver_log.h driver_log.cpp)

target_include_directories(driver_utils PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(driver_utils PRIVATE OpenVR::OpenVR)