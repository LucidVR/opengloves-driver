find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenVR REQUIRED)

add_library(driver_external_services STATIC tracking_reference_discovery.h tracking_reference_discovery.cpp)

target_include_directories(driver_external_services PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(driver_external_services PUBLIC OpenVR::OpenVR)
target_link_libraries(driver_external_services PRIVATE driver-includes opengloves_protos driver_utils)