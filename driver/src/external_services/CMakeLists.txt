find_package(OpenVR REQUIRED)
find_package(Crow CONFIG REQUIRED)

add_library(driver_external_services STATIC driver_internal.h driver_internal.cpp)

target_include_directories(driver_external_services PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(driver_external_services PUBLIC OpenVR::OpenVR)
target_link_libraries(driver_external_services PRIVATE driver-includes driver_utils Crow::Crow)