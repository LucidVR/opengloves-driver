find_package(OpenVR REQUIRED)

add_library(driver_utils STATIC driver_log.h driver_log.cpp file_path.h driver_math.h driver_math.cpp win_util.h)

if (WIN32)
    target_sources(driver_utils PRIVATE win_util.cpp file_path_win.cpp)
elseif(UNIX AND NOT APPLE)
    target_sources(driver_utils PRIVATE file_path_linux.cpp)
endif()

target_include_directories(driver_utils PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(driver_utils PRIVATE OpenVR::OpenVR)