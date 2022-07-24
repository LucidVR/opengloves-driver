add_library(communication-includes INTERFACE)

target_include_directories(communication-includes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(probers)
add_subdirectory(services)