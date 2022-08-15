find_package(OpenVR REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)


add_library(device_configuration STATIC device_configuration.h device_configuration.cpp)


target_link_libraries(device_configuration PUBLIC opengloves_server-includes OpenVR::OpenVR nlohmann_json::nlohmann_json)
target_link_libraries(device_configuration PRIVATE driver-includes driver_utils)