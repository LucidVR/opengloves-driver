add_library(device-includes INTERFACE)

target_include_directories(device-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(configuration)
add_subdirectory(drivers)
add_subdirectory(providers)