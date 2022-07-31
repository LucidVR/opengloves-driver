add_library(communication-includes INTERFACE)

target_include_directories(communication-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(probers)
add_subdirectory(services)
add_subdirectory(encoding)
add_subdirectory(managers)