add_library(encoding_services STATIC
        encoding_service.h
        alpha/alpha_encoding_service.h
        alpha/alpha_encoding_service.cpp
        )

set_target_properties(encoding_services PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(encoding_services PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(encoding_services PRIVATE opengloves_server-includes)